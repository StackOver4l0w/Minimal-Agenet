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
#include "logger.h"      /* LOG_ERROR */
#include "environment.h"
#include "string.h"

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
/* naked functions have no prologue, but a bare asm statement needs a
 * body; a top-level asm keeps the symbol portable across gcc and clang
 * (llvm-mingw warns on naked+asm in C mode). */
void ___chkstk_ms(void);
asm(
    ".globl ___chkstk_ms\n"
    "___chkstk_ms:\n"
    "   pushq %rcx\n"
    "   pushq %rax\n"
    "   cmpq  $0x1000, %rax\n"
    "   leaq  0x18(%rsp), %rcx\n"
    "   jb    2f\n"
    "1:\n"
    "   subq  $0x1000, %rcx\n"
    "   orq   $0, (%rcx)\n"      /* touch: commits the guard page */
    "   subq  $0x1000, %rax\n"
    "   cmpq  $0x1000, %rax\n"
    "   ja    1b\n"
    "2:\n"
    "   subq  %rax, %rcx\n"
    "   orq   $0, (%rcx)\n"      /* final touch at the frame bottom */
    "   popq  %rax\n"
    "   popq  %rcx\n"
    "   ret\n"
);
#endif

#define ENTRY_ARGC_MAX 8

/* Wide command line -> narrow argv. Classic tokenizer: remember where the
 * token started, close it with a NUL at the separator; double quotes
 * toggle "inside a token" so a quoted path survives - enough for the
 * agent's "<URL> [-v]" usage.
 * LIFETIME: argv[] points INTO the scratch buffer, so the buffer must
 * outlive this function - it lives on entry()'s frame and arrives as a
 * parameter. Never make it local to this function: after it returns, a
 * local buffer is dead memory and every argv string is garbage. */
static INT32 split_command_line(const PWCHAR cmdline, CHAR *argv[], INT32 argv_max,
                                 CHAR narrow[], USIZE narrow_size)
{
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
            if (token == NULL && o + 1 < narrow_size)
                token = &narrow[o];
            if (o + 1 < narrow_size)
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


__attribute__((section(".text.startup"), used))
void entry(void)
{
    /*      * The KERNEL32 table lives on entry's frame: this frame outlives the
     * whole agent (agent_main returns here, then we exit the process), so
     * a local is as permanent as a static - without static storage. */
    KERNEL32 entry_k32;
    if (!KERNEL32_Ctor(&entry_k32))
        return;                            /* no table - no way out  */

    PPEB peb = GetCurrentPEB();
    PWCHAR cmdline = peb->ProcessParameters->CommandLine.Buffer;

    /* The narrow command line - argv[] points into it (see above). */
    CHAR url_arg[2048];
    if (GetVariable("URL", url_arg, sizeof(url_arg)) == 0) {
        LOG_ERROR("Environment variable URL not set\n");
        return;
    }

    WCHAR url_arg_w[2048];
    if (AnsiToWide(url_arg, url_arg_w, 2048) < 0) {
        LOG_ERROR("Environment variable URL is invalid\n");
        return;
    }

    INT32 rc = agent_main(url_arg_w);
    entry_k32.ExitProcess((UINT32)rc);
}