/* logger.c - the one stdout writer (see logger.h).
 *
 * PRINT_FORMATTED_STRING formats into a stack buffer with our own
 * FormatV (no vsnprintf - there is no libc), then hands the bytes
 * to WriteFile on the stdout handle: unbuffered, one syscall per
 * line. The KERNEL32 table is rebuilt per call - this is a dev-only
 * path, and in a release build this whole file is dead code (the
 * macros expand to nothing and the linker drops the rest).
 */

#include "logger.h"
#include "types.h"
#include "string.h"
#include "kernel32.h"

#define STD_OUTPUT_HANDLE  ((DWORD)-11)

INT32 PRINT_FORMATTED_STRING(PCHAR format, ...)
{
    /* One log line fits in 1 KB; the frame pays for it, nothing static. */
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