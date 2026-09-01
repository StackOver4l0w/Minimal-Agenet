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

/* --- stack probe compatibility ----------------------------------------
 * Modern x64 Windows toolchains can emit `__chkstk` for large stack frames,
 * while older mingw/libgcc variants used `___chkstk_ms`. Linkers accept
 * either name; the CRT-free build needs both to satisfy all toolchains.
 *
 * Contract: size arrives in RAX, RCX walks the stack downward touching page
 * boundaries so the OS commits the guard page legally, and the caller's
 * prologue still performs the actual frame allocation. */
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
__attribute__((naked))
void __alloca(void)
{
    __asm__ volatile(
        "movl 0(%esp), %ecx\n"
        "addl $15, %eax\n"
        "andl $-16, %eax\n"
        "subl %eax, %esp\n"
        "movl %esp, %eax\n"
        "jmp *%ecx\n"
    );
}
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