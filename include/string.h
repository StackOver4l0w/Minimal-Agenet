#pragma once
#include "types.h"

INT32 AnsiToWide(const CHAR *ansi, PWCHAR wide, INT32 wideSize);
INT32 strcmp(const CHAR *s1, const CHAR *s2);
INT32 FormatV(PCHAR s, PCHAR format, va_list args);
__SIZE_TYPE__ strlen(const CHAR *s);
__SIZE_TYPE__ wcslen(const WCHAR *s);
