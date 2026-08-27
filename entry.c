/* entry.c - see entry.h.
 *
 * Everything here exists because -nostdlib removed the CRT startup
 * that used to do it silently. Order in entry() is load-bearing:
 * .bss first (every static "resolved yet?" flag reads garbage until
 * zeroed), then the KERNEL32 table, then PEB reads, then the agent.
 */

#include "entry.h"
#include "peb.h"
#include "kernel32.h"
#include "memory.h"
#include "system.h"      /* IMAGE_DOS_HEADER_MIN + signatures */

/* --- ___chkstk_ms: the stack probe ------------------------------------
 * gcc emits `call ___chkstk_ms` (three underscores on mingw x64) in any
 * function whose frame exceeds one page - our 72 KB run_session frame
 * qualifies. Transcribed VERBATIM from libgcc's _chkstk_ms.o (the
 * canonical implementation; an earlier hand-rolled variant misread the
 * contract - size arrives in RAX, not R10 - and crashed on the first
 * big frame):
 *   - RAX = byte count needed (set by the caller's prologue)
 *   - RCX walks DOWN from lea [rsp+0x18] (two pushes + return addr),
 *     one page per step; `orq 0,(rcx)` touches the page so the OS
 *     commits the guard page and grows the stack legally
 *   - RSP is never moved: the caller's "sub rsp, rax" does the alloc */
#if defined(ENVIRONMENT_x86_64) || defined(__x86_64__) || defined(_M_X64)
__attribute__((naked, used))
void ___chkstk_ms(void)
{
    __asm__ volatile (
        "pushq %rcx\n\t"
        "pushq %rax\n\t"
        "cmpq  $0x1000, %rax\n\t"
        "leaq  0x18(%rsp), %rcx\n\t"
        "jb    2f\n\t"
        "1:\n\t"
        "subq  $0x1000, %rcx\n\t"
        "orq   $0, (%rcx)\n\t"      /* touch: commits the guard page */
        "subq  $0x1000, %rax\n\t"
        "cmpq  $0x1000, %rax\n\t"
        "ja    1b\n\t"
        "2:\n\t"
        "subq  %rax, %rcx\n\t"
        "orq   $0, (%rcx)\n\t"      /* final touch at the frame bottom */
        "popq  %rax\n\t"
        "popq  %rcx\n\t"
        "ret\n\t"
    );
}
#endif

#define ENTRY_ARGC_MAX 8

static KERNEL32 entry_k32;

/* Wide command line -> narrow argv over a static scratch (statics are
 * legal until the PIC phase). Classic tokenizer: remember where the
 * token started, close it with a NUL at the separator; double quotes
 * toggle "inside a token" so a quoted path survives - enough for the
 * agent's "<URL> [-v]" usage. */
static INT32 split_command_line(const PWCHAR cmdline, CHAR *argv[], INT32 argv_max)
{
    static CHAR narrow[2048];
    INT32 argc = 0;
    USIZE o = 0;
    BOOL in_quotes = FALSE;
    CHAR *token = NULL;

    for (PWCHAR p = cmdline; ; p++) {
        WCHAR c = *p;

        if (c == L'"') {
            in_quotes = !in_quotes;
            continue;
        }

        BOOL separator = (c == L' ' && !in_quotes) || c == L'\0';

        if (!separator) {
            if (token == NULL && o + 1 < sizeof(narrow))
                token = &narrow[o];
            if (o + 1 < sizeof(narrow))
                narrow[o++] = (CHAR)(c & 0xFF);
        } else if (token != NULL) {
            narrow[o++] = '\0';
            if (argc < argv_max)
                argv[argc++] = token;
            token = NULL;
        }

        if (c == L'\0')
            break;
    }

    return argc;
}

/* Zero .bss by walking our own PE section table - no reliance on
 * linker-provided __bss_start/__bss_end (those come from the CRT link
 * scripts; under -nostdlib nobody guarantees them). The image base
 * comes from the PEB; from there it is the standard header walk:
 * DOS header -> e_lfanew -> PE sig -> file header (section count,
 * optional-header size) -> section table. .bss is the section whose
 * raw size is zero on disk but whose virtual size is the bytes the
 * loader maps as zero-initialized - except nothing actually zeroes it
 * for us, which is exactly why this function exists. */
static void zero_bss_from_pe(void)
{
    PPEB peb = GetCurrentPEB();
    PUINT8 base = (PUINT8)peb->ImageBase;

    PIMAGE_DOS_HEADER_MIN dos = (PIMAGE_DOS_HEADER_MIN)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE_MIN)
        return;                            /* not a PE - nothing to do */

    PUINT8 nt = base + dos->e_lfanew;
    if (*(PUINT32)nt != IMAGE_NT_SIGNATURE_MIN)
        return;

    PUINT8 file_header = nt + 4;                    /* IMAGE_FILE_HEADER */
    UINT16 num_sections = *(PUINT16)(file_header + 2);
    UINT16 opt_size     = *(PUINT16)(file_header + 16);
    PUINT8 sections     = file_header + 20 + opt_size;  /* 40 bytes each */

    for (UINT16 i = 0; i < num_sections; i++) {
        PUINT8 sec = sections + (USIZE)i * 40;
        const CHAR *name = (const CHAR *)sec;       /* 8 bytes, may be unNULed */
        UINT32 virtual_size = *(PUINT32)(sec + 8);
        UINT32 virtual_addr = *(PUINT32)(sec + 12);

        if (name[0]=='.' && name[1]=='b' && name[2]=='s' &&
            name[3]=='s' && name[4]=='\0') {
            MemoryZero(base + virtual_addr, virtual_size);
            return;
        }
    }
}

__attribute__((section(".text.startup"), used))
void entry(void)
{
    /* .bss first - every static "resolved yet?" flag reads garbage
     * until zeroed, and the first NULL-check on garbage takes the
     * wrong branch (a call through a garbage pointer). */
    zero_bss_from_pe();

    if (!KERNEL32_Ctor(&entry_k32))
        return;                            /* no table - no way out  */

    PPEB peb = GetCurrentPEB();
    PWCHAR cmdline = peb->ProcessParameters->CommandLine.Buffer;

    CHAR *argv[ENTRY_ARGC_MAX];
    INT32 argc = split_command_line(cmdline, argv, ENTRY_ARGC_MAX);

    INT32 rc = agent_main(argc, argv);
    entry_k32.ExitProcess((UINT32)rc);
}
