/* memory.h - memory primitives without libc.
 *
 * MemoryZero / MemoryCopy are the project's own names; memset is the
 * C-standard name the compiler is allowed to emit on its own (see
 * memory.c for why it must exist even though nobody calls it directly).
 *
 * memset's size uses __SIZE_TYPE__ (the compiler's own size_t): the
 * builtin's signature must match EXACTLY or clang warns about an
 * incompatible redeclaration (on i386 its size_t is unsigned int,
 * not our 64-bit SIZE_T typedef).
 */

#include "types.h"

void MemoryZero(void *ptr, SIZE_T size);
void MemoryCopy(void *dest, const void *src, SIZE_T size);
void *memset(void *dest, int value, __SIZE_TYPE__ count);
