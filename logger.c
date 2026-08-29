#include "logger.h"
#include "types.h"
#include "string.h"
#include "kernel32.h"

#define STD_OUTPUT_HANDLE  ((DWORD)-11)

INT32 PRINT_FORMATTED_STRING(PCHAR format, ...)
{
    /* С1 step 2: stack-local scratch (a static buffer is .bss). One
     * line of log fits in 1 KB; deeper frames pay the probe, not .bss. */
    CHAR buffer[1024];
    va_list args;

    va_start(args, format);
    INT32 len = FormatV(buffer, format, args);
    va_end(args);

    if (len < 0)
        return -1;

    KERNEL32 kernel;

    if (!KERNEL32_Ctor(&kernel))
        return -1;

    HANDLE stdout_handle = kernel.GetStdHandle(STD_OUTPUT_HANDLE);

    if (!stdout_handle)
        return -1;

    DWORD written = 0;

    if (!kernel.WriteFile(stdout_handle, buffer, (DWORD)len, &written, NULL)){
        return -1;
    }

    return (INT32)written;
}