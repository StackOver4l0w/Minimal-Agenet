/* freestanding.c - the compiler-builtin supplier.
 *
 * Why this file exists: with -nostdlib the compiler is STILL free to
 * emit calls to memcpy/memset/strlen-family - the C standard's
 * freestanding clause explicitly reserves these four (mem*) and
 * compilers go beyond (strlen/wcslen) when they recognize loop idioms
 * at -O2. GCC and clang recognize DIFFERENT sets, so chasing each
 * pattern with volatile (the memory.c/string.c treatment) is a race
 * against every compiler version. Instead we do what the standard
 * expects of a freestanding implementation: provide them.
 *
 * Each body is a volatile loop - the same idiom-breaking discipline as
 * memory.c - so no recursive recognition can occur, and none of these
 * pull CRT code into the image: they compile to plain byte loops.
 * The empty-import-table property is untouched (these are our own
 * definitions, resolved at link time like any other function).
 */

#include "freestanding.h"

void *memset(void *dst, INT32 c, USIZE n)
{
    volatile unsigned char *d = (volatile unsigned char *)dst;
    for (USIZE i = 0; i < n; i++)
        d[i] = (unsigned char)c;
    return dst;
}

void *memcpy(void *dst, const void *src, USIZE n)
{
    volatile unsigned char *d = (volatile unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    for (USIZE i = 0; i < n; i++)
        d[i] = s[i];
    return dst;
}

void *memmove(void *dst, const void *src, USIZE n)
{
    volatile unsigned char *d = (volatile unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    if (d < s) {
        for (USIZE i = 0; i < n; i++)
            d[i] = s[i];
    } else {
        for (USIZE i = n; i > 0; i--)
            d[i - 1] = s[i - 1];
    }
    return dst;
}

INT32 memcmp(const void *a, const void *b, USIZE n)
{
    const volatile unsigned char *x = (const volatile unsigned char *)a;
    const volatile unsigned char *y = (const volatile unsigned char *)b;
    for (USIZE i = 0; i < n; i++) {
        if (x[i] != y[i])
            return (INT32)x[i] - (INT32)y[i];
    }
    return 0;
}

USIZE strlen(const CHAR *s)
{
    const volatile CHAR *p = s;
    USIZE n = 0;
    while (*p++) n++;
    return n;
}

USIZE wcslen(const WCHAR *s)
{
    const volatile WCHAR *p = s;
    USIZE n = 0;
    while (*p++) n++;
    return n;
}
