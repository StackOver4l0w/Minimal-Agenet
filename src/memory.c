#include "memory.h"

void *memset(void *dest, int value, __SIZE_TYPE__ count)
{
    volatile unsigned char *p = (volatile unsigned char *)dest;

    while (count--)
        *p++ = (unsigned char)value;

    return dest;
}

void MemoryZero(void *ptr, SIZE_T size)
{
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    for (SIZE_T i = 0; i < size; i++)
        p[i] = 0;
}

void MemoryCopy(void *dest, const void *src, SIZE_T size)
{
    volatile unsigned char *d = (volatile unsigned char *)dest;
    const volatile unsigned char *s = (const volatile unsigned char *)src;
    for (SIZE_T i = 0; i < size; i++)
        d[i] = s[i];
}
