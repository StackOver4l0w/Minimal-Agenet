/* ntdll.c - the ntdll function table (see ntdll.h).
 *
 * ntdll IS the loader, so it is mapped before any user code runs -
 * the PEB walk always finds it. Two exports matter to this agent:
 * LdrLoadDll (to map winhttp.dll, which nothing else loads for us)
 * and RtlGetVersion (the OS version that does not lie - the classic
 * GetVersionEx is patched per-manifest).
 */

#include "ntdll.h"
#include "djb2.h"
#include "peb.h"
#include "system.h"
#include "apihash.h"

BOOL NTDLL_Ctor(NTDLL *ntdll)
{
    if (ntdll == NULL)
        return FALSE;

    ntdll->LdrLoadDll = (NTSTATUS (WINAPI *)(WCHAR *, UINT32, PUNICODE_STRING, PVOID *))
        ResolveFromModuleByHash(HASH_MOD_NTDLL, HASH_LDRLOADDLL);
    ntdll->RtlGetVersion = (NTSTATUS (WINAPI *)(PVOID))
        ResolveFromModuleByHash(HASH_MOD_NTDLL, HASH_RTLGETVERSION);

    return (ntdll->LdrLoadDll != NULL && ntdll->RtlGetVersion != NULL);
}
