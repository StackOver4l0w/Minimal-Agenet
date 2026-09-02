#include "types.h"

void MemoryZero(void *ptr, SIZE_T size);
void MemoryCopy(void *dest, const void *src, SIZE_T size);
void *memset(void *dest, int value, __SIZE_TYPE__ count);
