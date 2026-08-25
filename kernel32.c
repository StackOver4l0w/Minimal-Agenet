#include "kernel32.h"
#include "system.h"

BOOL KERNEL32_Ctor(KERNEL32 *kernel)
{
    if (kernel == NULL)
        return FALSE;

    kernel->GetProcAddress = (PVOID (WINAPI *)(PVOID, const CHAR *))
        ResolveFromModuleByName(L"kernel32.dll", "GetProcAddress");
    kernel->LoadLibraryA = (PVOID (WINAPI *)(const CHAR *))
        ResolveFromModuleByName(L"kernel32.dll", "LoadLibraryA");
    kernel->GetComputerNameA = (BOOL (WINAPI *)(PCHAR, DWORD *))
        ResolveFromModuleByName(L"kernel32.dll", "GetComputerNameA");
    kernel->SetHandleInformation = (BOOL (WINAPI *)(HANDLE, DWORD, DWORD))
        ResolveFromModuleByName(L"kernel32.dll", "SetHandleInformation");
    kernel->CreateProcessW = (BOOL (WINAPI *)(const PWCHAR, const PWCHAR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, PVOID, const PWCHAR, LPSTARTUPINFOW, LPPROCESS_INFORMATION))
        ResolveFromModuleByName(L"kernel32.dll", "CreateProcessW");
    kernel->CloseHandle = (BOOL (WINAPI *)(HANDLE))
        ResolveFromModuleByName(L"kernel32.dll", "CloseHandle");
    kernel->TerminateProcess = (BOOL (WINAPI *)(HANDLE, UINT32))
        ResolveFromModuleByName(L"kernel32.dll", "TerminateProcess");

    return (kernel->GetProcAddress != NULL &&
            kernel->LoadLibraryA != NULL &&
            kernel->GetComputerNameA != NULL &&
            kernel->SetHandleInformation != NULL &&
            kernel->CreateProcessW != NULL &&
            kernel->CloseHandle != NULL &&
            kernel->TerminateProcess != NULL);
}