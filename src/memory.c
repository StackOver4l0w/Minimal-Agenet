/* memory.c - the three memory primitives, hand-rolled (see memory.h).
 *
 * Why they exist at all: under -nostdlib there is no libc, but the
 * compiler still EMITS calls to memset/memcpy when it recognizes a
 * loop idiom (structure zeroing, buffer copies) - so these symbols
 * must exist or the link fails.
 *
 * Why every pointer is volatile: without it, at -O2 the optimizer
 * recognizes these loops as memset/memcpy idioms and replaces them
 * with... a call to memset/memcpy. Recursion through the toolchain.
 * Volatile forces each iteration's load and store to exist as real
 * instructions. (The volatile must be on the DECLARED pointer, not on
 * a cast at the use site - compilers routinely look through casts.)
 *
 * Byte-at-a-time is the simple, correct baseline. A word-at-a-time or
 * rep movsb version is faster; speed has not been needed yet, and the
 * plain loops have no recognizable byte signature for scanners.
 */

#include "memory.h"

void *memset(void *dest, int value, SIZE_T count)
{
    volatile unsigned char *p = (volatile unsigned char *)dest;

    while (count--)
        *p++ = (unsigned char)value;

    return dest;
}

/* Zero a region. Used on structures that must start clean (URL_COMPONENTS,
 * PROCESS_INFORMATION, the shell pool slots...). */
void MemoryZero(void *ptr, SIZE_T size)
{
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    for (SIZE_T i = 0; i < size; i++)
        p[i] = 0;
}

/* Copy a region. The two loops share volatile discipline with memset. */
void MemoryCopy(void *dest, const void *src, SIZE_T size)
{
    volatile unsigned char *d = (volatile unsigned char *)dest;
    const volatile unsigned char *s = (const volatile unsigned char *)src;
    for (SIZE_T i = 0; i < size; i++)
        d[i] = s[i];
}
