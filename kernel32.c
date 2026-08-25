#include "kernel32.h"
#include "system.h"

BOOL KERNEL_Ctor(KERNEL *kernel)
{
    if (kernel == NULL)
        return FALSE;

    kernel->GetProcAddress = (PVOID (WINAPI *)(PVOID, const CHAR *))
        ResolveFromModuleByName(L"kernel32.dll", "GetProcAddress");
    kernel->LoadLibraryA = (PVOID (WINAPI *)(const CHAR *))
        ResolveFromModuleByName(L"kernel32.dll", "LoadLibraryA");
    kernel->GetComputerNameA = (BOOL (WINAPI *)(PCHAR, DWORD *))
        ResolveFromModuleByName(L"kernel32.dll", "GetComputerNameA");

    return (kernel->GetProcAddress != NULL &&
            kernel->LoadLibraryA != NULL &&
            kernel->GetComputerNameA != NULL);
}