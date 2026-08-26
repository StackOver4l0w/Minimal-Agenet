#include "memory.h"

void MemoryZero(void *ptr, SIZE_T size)
{
    unsigned char *p = (unsigned char *)ptr;
    for (SIZE_T i = 0; i < size; i++)
        p[i] = 0;
}

void MemoryCopy(void *dest, const void *src, SIZE_T size)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    for (SIZE_T i = 0; i < size; i++)
        d[i] = s[i];
}