/* memory.h - memory primitives without libc.
 *
 * MemoryZero / MemoryCopy are the project's own names; memset is the
 * C-standard name the compiler is allowed to emit on its own (see
 * memory.c for why it must exist even though nobody calls it directly).
 */

#include "types.h"

void MemoryZero(void *ptr, SIZE_T size);
void MemoryCopy(void *dest, const void *src, SIZE_T size);
void *memset(void *dest, int value, SIZE_T count);
