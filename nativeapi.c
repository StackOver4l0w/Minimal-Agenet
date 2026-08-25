#include "nativeapi.h"

BOOL NativeApi_Ctor(NTDLL *ntdll, KERNEL32 *kernel){
    return NTDLL_Ctor(ntdll) && KERNEL32_Ctor(kernel);
}