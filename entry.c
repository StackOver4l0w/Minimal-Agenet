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
asm(
    ".globl __chkstk\n"
    ".globl ___chkstk_ms\n"
    "__chkstk:\n"
    "___chkstk_ms:\n"
    "   pushq %rcx\n"
    "   pushq %rax\n"
    "   cmpq  $0x1000, %rax\n"
    "   leaq  0x18(%rsp), %rcx\n"
    "   jb    2f\n"
    "1:\n"
    "   subq  $0x1000, %rcx\n"
    "   orq   $0, (%rcx)\n"
    "   subq  $0x1000, %rax\n"
    "   cmpq  $0x1000, %rax\n"
    "   ja    1b\n"
    "2:\n"
    "   subq  %rax, %rcx\n"
    "   orq   $0, (%rcx)\n"
    "   popq  %rax\n"
    "   popq  %rcx\n"
    "   ret\n"
);
#elif defined(ENVIRONMENT_I386) || defined(__i386__) || defined(_M_IX86)
/* i386 clang emits `call __alloca` for frames over one page - the 32-bit
 * chkstk twin. Contract (same family as the x64 probe above): EAX = byte
 * count on entry, walk DOWN one page at a time touching each (the OS
 * commits the guard page), SP is never moved - the caller's "sub esp"
 * after the call does the allocation. Top-level asm, not naked+asm:
 * llvm-mingw warns on naked C functions. */
asm(
    ".globl __alloca\n"
    "__alloca:\n"
    "   pushl %ecx\n"
    "   pushl %eax\n"
    "   leal 0x0c(%esp), %ecx\n"
    "   cmpl $0x1000, %eax\n"
    "   jb 2f\n"
    "1:\n"
    "   subl $0x1000, %ecx\n"
    "   orl $0, (%ecx)\n"
    "   subl $0x1000, %eax\n"
    "   cmpl $0x1000, %eax\n"
    "   ja 1b\n"
    "2:\n"
    "   subl %eax, %ecx\n"
    "   orl $0, (%ecx)\n"
    "   popl %eax\n"
    "   popl %ecx\n"
    "   ret\n"
);
#elif defined(ENVIRONMENT_ARM64) || defined(__aarch64__) || defined(_M_ARM64)
/* aarch64 clang emits `bl __chkstk` for big frames. Windows ARM64 ABI
 * contract: X15 = byte count on entry; probe DOWN page by page with a
 * store (commits each guard page); SP untouched - the caller subtracts. */
asm(
    ".globl __chkstk\n"
    "__chkstk:\n"
    "   mov  x16, sp\n"
    "   bfc  x16, #0, #12\n"
    "1:  sub  x16, x16, #0x1000\n"
    "   str  xzr, [x16]\n"
    "   subs x15, x15, #0x1000\n"
    "   b.hi 1b\n"
    "   ret\n"
);
#endif

#define ENTRY_ARGC_MAX 8


__attribute__((section(".text.startup"), used))
void entry(void)
{
    KERNEL32 entry_k32;
    if (!KERNEL32_Ctor(&entry_k32))
        return;   

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