/* winhttp_api.h - runtime-resolved WinHTTP function table (no IAT import).
 *
 * Pattern twin of kernel32.h / ntdll.h / advapi.h: a struct of function
 * pointers filled at runtime by walking the PEB loader list and the
 * module's export table (system.h: ResolveFromModuleByName). The linker
 * never sees a call to these exports, so the name never enters the IAT -
 * the exe stops importing it.
 *
 * Named winhttp_api (not winhttp) on purpose: the system header is
 * <winhttp.h>; a local file with the same name would make "which one got
 * included" ambiguous (quotes search locally, angle brackets the system).
 *
 * No <windows.h>: every type this header needs (HINTERNET, DWORD, BOOL,
 * const WCHAR *, UINT16, ULONG_PTR, PVOID and the WebSocket buffer-type enum)
 * lives in types.h / wintypes.h - the whole point of this codebase's
 * minimal typedef dictionary.
 *
 * Bootstrap note (this ref-based branch): the exe still carries the
 * other WinHTTP imports, so winhttp.dll is guaranteed in the PEB list
 * at startup - a plain ResolveFromModuleByName finds it. When the last
 * static import dies, the Ctor grows an LdrLoadDll bootstrap first.
 */

#pragma once

#include "types.h"
#include "wintypes.h"

/* --- WinHTTP constants, values from <winhttp.h> (verified on disk) ------
 * Copied so the system header can be dropped. Each cites its SDK name. */
#define WINHTTP_ACCESS_TYPE_DEFAULT_PROXY 0        /* no-proxy default   */
#define WINHTTP_FLAG_BYPASS_PROXY_CACHE   0x0100   /* == FLAG_REFRESH    */
#define WINHTTP_FLAG_REFRESH              WINHTTP_FLAG_BYPASS_PROXY_CACHE
#define WINHTTP_FLAG_SECURE               0x00800000
#define WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET 114   /* SDK: 114 (not 79)  */

/* NULL-sentinels; the SDK defines each as NULL for the parameter it guards. */
#define WINHTTP_NO_PROXY_NAME             NULL
#define WINHTTP_NO_PROXY_BYPASS           NULL
#define WINHTTP_NO_REFERER                NULL
#define WINHTTP_DEFAULT_ACCEPT_TYPES      NULL
#define WINHTTP_NO_ADDITIONAL_HEADERS     NULL
#define WINHTTP_NO_REQUEST_DATA           NULL

/* URL_COMPONENTS + scheme ids for WinHttpCrackUrl (SDK winhttp.h shape,
 * composed of types.h types only). */
typedef struct _URL_COMPONENTS
{
    DWORD           dwStructSize;
    const WCHAR    *lpszScheme;
    DWORD           dwSchemeLength;
    INT32           nScheme;
    const WCHAR    *lpszHostName;
    DWORD           dwHostNameLength;
    UINT16          nPort;
    const WCHAR    *lpszUserName;
    DWORD           dwUserNameLength;
    const WCHAR    *lpszPassword;
    DWORD           dwPasswordLength;
    const WCHAR    *lpszUrlPath;
    DWORD           dwUrlPathLength;
    const WCHAR    *lpszExtraInfo;
    DWORD           dwExtraInfoLength;
} URL_COMPONENTS;

#define INTERNET_SCHEME_HTTP               1
#define INTERNET_SCHEME_HTTPS              2

typedef struct WINHTTP_API
{
    /* BOOL WINAPI WinHttpCrackUrl(url, len, flags, components)
     * Classic contract: BOOL + GetLastError. */
    BOOL (WINAPI *WinHttpCrackUrl)(const WCHAR *pwszUrl,
                                   DWORD dwUrlLength,
                                   DWORD dwFlags,
                                   URL_COMPONENTS *lpUrlComponents);

    /* DWORD WINAPI WinHttpWebSocketSend(hSocket, bufferType, buffer, length)
     *
     * WebSocket-family error contract: unlike classic WinHTTP calls
     * (BOOL + GetLastError), this family returns the error directly;
     * 0 = success. The signature is copied verbatim from <winhttp.h>. */
    DWORD (WINAPI *WinHttpWebSocketSend)(HINTERNET hWebSocket,
                                         WINHTTP_WEB_SOCKET_BUFFER_TYPE eBufferType,
                                         PVOID pvBuffer,
                                         DWORD dwBufferLength);

    /* DWORD WINAPI WinHttpWebSocketReceive(hSocket, buffer, len, got, type)
     * Same direct-error contract. */
    DWORD (WINAPI *WinHttpWebSocketReceive)(HINTERNET hWebSocket,
                                            PVOID pvBuffer,
                                            DWORD dwBufferLength,
                                            DWORD *pdwBytesRead,
                                            WINHTTP_WEB_SOCKET_BUFFER_TYPE *peBufferType);

    /* The eight classic calls: BOOL + GetLastError. */
    HINTERNET (WINAPI *WinHttpOpen)(const WCHAR * pszAgentW,
                                    DWORD dwAccessType,
                                    const WCHAR * pszProxyW,
                                    const WCHAR * pszProxyBypassW,
                                    DWORD dwFlags);

    HINTERNET (WINAPI *WinHttpConnect)(HINTERNET hSession,
                                       const WCHAR * pswzServerName,
                                       UINT16 nServerPort,
                                       DWORD dwReserved);

    HINTERNET (WINAPI *WinHttpOpenRequest)(HINTERNET hConnect,
                                           const WCHAR * pswzVerb,
                                           const WCHAR * pswzObject,
                                           const WCHAR * pswzVersion,
                                           const WCHAR * pswzReferrer,
                                           const WCHAR * *ppszAcceptTypes,
                                           DWORD dwFlags);

    BOOL (WINAPI *WinHttpSetOption)(HINTERNET hInternet,
                                    DWORD dwOption,
                                    PVOID lpBuffer,
                                    DWORD dwBufferLength);

    BOOL (WINAPI *WinHttpSendRequest)(HINTERNET hRequest,
                                      const WCHAR * lpszHeaders,
                                      DWORD dwHeadersLength,
                                      PVOID lpOptional,
                                      DWORD dwOptionalLength,
                                      DWORD dwTotalLength,
                                      ULONG_PTR dwContext);

    BOOL (WINAPI *WinHttpReceiveResponse)(HINTERNET hRequest,
                                          PVOID lpReserved);

    HINTERNET (WINAPI *WinHttpWebSocketCompleteUpgrade)(HINTERNET hRequest,
                                                        ULONG_PTR dwContext);

    BOOL (WINAPI *WinHttpCloseHandle)(HINTERNET hInternet);
} WINHTTP_API;

/* Resolve every member. FALSE if any step failed; callers treat a NULL
 * member as "not initialized yet" and retry lazily (transport.c). */
BOOL WINHTTP_API_Ctor(WINHTTP_API *api);
