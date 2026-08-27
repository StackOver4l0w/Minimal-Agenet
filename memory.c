#include "memory.h"

/* The pointers are volatile on purpose: at -O2 GCC recognizes plain
 * fill/copy loops as memset/memcpy idioms and REPLACES them with calls
 * to the CRT functions - which do not exist under -nostdlib (undefined
 * reference). volatile makes every byte access an observable event the
 * optimizer must keep as written. */

void MemoryZero(void *ptr, SIZE_T size)
{
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    for (SIZE_T i = 0; i < size; i++)
        p[i] = 0;
}

void MemoryCopy(void *dest, const void *src, SIZE_T size)
{
    volatile unsigned char *d = (volatile unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    for (SIZE_T i = 0; i < size; i++)
        d[i] = s[i];
}