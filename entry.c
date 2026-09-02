#include "entry.h"
#include "peb.h"
#include "kernel32.h"
#include "memory.h"
#include "system.h"
#include "logger.h"
#include "environment.h"
#include "string.h"
#include "stackstrings.h"

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

    CHAR env_name[8];
    StrEnvUrl(env_name);

    CHAR url_arg[2048];
    if (GetVariable(env_name, url_arg, sizeof(url_arg)) == 0) {
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
