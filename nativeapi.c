#include "nativeapi.h"

BOOL NativeApi_Ctor(NTDLL *ntdll, KERNEL *kernel){
    return NTDLL_Ctor(ntdll) && KERNEL_Ctor(kernel);
}