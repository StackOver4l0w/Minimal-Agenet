/* freestanding.h - the compiler-builtin supplier (see freestanding.c). */

#ifndef FREESTANDING_H
#define FREESTANDING_H

#include "types.h"

/* The set the optimizer is allowed to emit even under -nostdlib
 * (C standard, freestanding clause). Provided here so ANY compiler -
 * gcc today, clang/llvm-mingw in CI - finds them satisfied. */
void  *memset(void *dst, INT32 c, USIZE n);
void  *memcpy(void *dst, const void *src, USIZE n);
void  *memmove(void *dst, const void *src, USIZE n);
INT32  memcmp(const void *a, const void *b, USIZE n);
USIZE  strlen(const CHAR *s);
USIZE  wcslen(const WCHAR *s);

#endif /* FREESTANDING_H */
