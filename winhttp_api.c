/* winhttp_api.c - the constructor behind winhttp_api.h.
 *
 * Bootstrap: with every WinHTTP call resolved at runtime, the exe no
 * longer imports winhttp.dll - so the loader never maps it and there
 * is nothing in the PEB list to walk. The Ctor asks the loader itself:
 * ntdll.dll is mapped in every process (it IS the loader), its
 * LdrLoadDll export maps winhttp.dll for us, and only then does the
 * table resolve. One ResolveFromModuleByName per member afterwards -
 * exactly the kernel32.c / ntdll.c pattern of this codebase.
 */

#include "winhttp_api.h"
#include "system.h"
#include "peb.h"
#include "ntdll.h"
#include "djb2.h"

/* Make sure winhttp.dll is mapped; return its base (NULL on failure). */
static PVOID map_winhttp(void)
{
    /* Already loaded? (Some other component may have pulled it in.) */
    PVOID base = GetModuleHandleFromPEB(Hash(L"winhttp.dll"));
    if (base != NULL)
        return base;

    /* Resolve LdrLoadDll from ntdll - the module itself must come from
     * the PEB walk; its name cannot be hashed before it is found. */
    NTDLL ntdll;
    if (!NTDLL_Ctor(&ntdll) || ntdll.LdrLoadDll == NULL)
        return NULL;

    UNICODE_STRING name;
    name.Length        = 22;             /* L"winhttp.dll": 11 chars * 2 */
    name.MaximumLength = 24;             /* + NUL                          */
    name.Buffer        = L"winhttp.dll";

    PVOID loaded = NULL;
    if (ntdll.LdrLoadDll(NULL, 0, &name, &loaded) != 0 || loaded == NULL)
        return NULL;

    return loaded;
}

BOOL WINHTTP_API_Ctor(WINHTTP_API *api)
{
    if (api == NULL)
        return FALSE;

    PVOID winhttp = map_winhttp();
    if (winhttp == NULL)
        return FALSE;

    api->WinHttpCrackUrl = (BOOL (WINAPI *)(const WCHAR *, DWORD, DWORD,
                                            URL_COMPONENTS *))
        ResolveExportByName(winhttp, "WinHttpCrackUrl");
    api->WinHttpWebSocketSend = (DWORD (WINAPI *)(HINTERNET,
                                                  WINHTTP_WEB_SOCKET_BUFFER_TYPE,
                                                  PVOID, DWORD))
        ResolveExportByName(winhttp, "WinHttpWebSocketSend");
    api->WinHttpWebSocketReceive = (DWORD (WINAPI *)(HINTERNET, PVOID, DWORD,
                                                     DWORD *,
                                                     WINHTTP_WEB_SOCKET_BUFFER_TYPE *))
        ResolveExportByName(winhttp, "WinHttpWebSocketReceive");
    api->WinHttpOpen = (HINTERNET (WINAPI *)(const WCHAR *, DWORD, const WCHAR *,
                                             const WCHAR *, DWORD))
        ResolveExportByName(winhttp, "WinHttpOpen");
    api->WinHttpConnect = (HINTERNET (WINAPI *)(HINTERNET, const WCHAR *, UINT16,
                                                DWORD))
        ResolveExportByName(winhttp, "WinHttpConnect");
    api->WinHttpOpenRequest = (HINTERNET (WINAPI *)(HINTERNET, const WCHAR *,
                                                    const WCHAR *, const WCHAR *, const WCHAR *,
                                                    const WCHAR **, DWORD))
        ResolveExportByName(winhttp, "WinHttpOpenRequest");
    api->WinHttpSetOption = (BOOL (WINAPI *)(HINTERNET, DWORD, PVOID, DWORD))
        ResolveExportByName(winhttp, "WinHttpSetOption");
    api->WinHttpSendRequest = (BOOL (WINAPI *)(HINTERNET, const WCHAR *, DWORD,
                                               PVOID, DWORD, DWORD,
                                               ULONG_PTR))
        ResolveExportByName(winhttp, "WinHttpSendRequest");
    api->WinHttpReceiveResponse = (BOOL (WINAPI *)(HINTERNET, PVOID))
        ResolveExportByName(winhttp, "WinHttpReceiveResponse");
    api->WinHttpWebSocketCompleteUpgrade = (HINTERNET (WINAPI *)(HINTERNET,
                                                                 ULONG_PTR))
        ResolveExportByName(winhttp, "WinHttpWebSocketCompleteUpgrade");
    api->WinHttpCloseHandle = (BOOL (WINAPI *)(HINTERNET))
        ResolveExportByName(winhttp, "WinHttpCloseHandle");

    return (api->WinHttpCrackUrl != NULL &&
            api->WinHttpWebSocketSend != NULL &&
            api->WinHttpWebSocketReceive != NULL &&
            api->WinHttpOpen != NULL &&
            api->WinHttpConnect != NULL &&
            api->WinHttpOpenRequest != NULL &&
            api->WinHttpSetOption != NULL &&
            api->WinHttpSendRequest != NULL &&
            api->WinHttpReceiveResponse != NULL &&
            api->WinHttpWebSocketCompleteUpgrade != NULL &&
            api->WinHttpCloseHandle != NULL);
}
