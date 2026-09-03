/* string.h - strings without libc.
 *
 * strlen/wcslen/strcmp reuse the C-standard names the compiler may
 * emit on its own; their signatures use __SIZE_TYPE__ so they match
 * the builtins exactly on every arch (clang warns otherwise).
 */

#include "types.h"

INT32 AnsiToWide(const CHAR *ansi, PWCHAR wide, INT32 wideSize);
INT32 strcmp(const CHAR *s1, const CHAR *s2);
INT32 FormatV(PCHAR s, PCHAR format, va_list args);
INT32 Format(PCHAR s, PCHAR format, ...);
__SIZE_TYPE__ strlen(const CHAR *s);
__SIZE_TYPE__ wcslen(const WCHAR *s);