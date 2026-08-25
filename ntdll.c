#include "ntdll.h"
#include "djb2.h"
#include "peb.h"
#include "system.h"

BOOL NTDLL_Ctor(NTDLL *ntdll)
{
    if (ntdll == NULL)
        return FALSE;

    ntdll->LdrLoadDll = (NTSTATUS (WINAPI *)(WCHAR *, UINT32, PUNICODE_STRING, PVOID *))
        ResolveFromModuleByName(L"ntdll.dll", "LdrLoadDll");
    ntdll->RtlGetVersion = (NTSTATUS (WINAPI *)(PVOID))
        ResolveFromModuleByName(L"ntdll.dll", "RtlGetVersion");

    return (ntdll->LdrLoadDll != NULL && ntdll->RtlGetVersion != NULL);
}
