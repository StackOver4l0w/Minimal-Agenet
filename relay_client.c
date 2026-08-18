/* relay_client - a receive-first WebSocket client for Windows (WinHTTP, 8+).
 *
 * The relay server (a WebSocket forwarder) never expects the client to speak
 * first: once the HTTP connection is upgraded to WebSocket, the SERVER sends
 * the first message (the first command). This client therefore connects,
 * completes the upgrade and then only LISTENS, printing every message it
 * receives to the terminal:
 *
 *   [1] connect to the URL given on the command line and upgrade to WebSocket
 *   [2] receive-first command loop - nothing is ever sent to the server
 *       (each message: WinHTTP buffer type, length, hex dump, printable text)
 *   [3] close the connection cleanly
 *
 *   Build (MinGW gcc):
 *       gcc -O2 -s -Wall -Wextra -o relay_client.exe relay_client.c -lwinhttp
 *   Run:
 *       relay_client.exe <URL>     (e.g. https://relay.example.com/agent)
 *
 * The URL is never hardcoded: it comes only from the command line, so the same
 * binary can talk to any WebSocket endpoint.
 *
 * How the WebSocket upgrade works in WinHTTP:
 *   - a normal HTTPS GET request is created (SECURE flag = TLS),
 *   - WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET tells WinHTTP to add the
 *     "Upgrade: websocket" handshake headers,
 *   - WinHttpSendRequest sends the handshake,
 *   - WinHttpWebSocketCompleteUpgrade finishes the handshake (the server
 *     answers "101 Switching Protocols") and returns a NEW handle that is
 *     used for all WebSocket send/receive calls. The old request handle
 *     is no longer needed and gets closed.
 *
 * NOTE: unlike the classic WinHTTP functions, the WinHttpWebSocket* family
 * does NOT return BOOL. It returns the error code directly (DWORD), so the
 * error checks here compare against NO_ERROR instead of using GetLastError().
 *
 * NOTE: WinHTTP does not expose raw RFC 6455 frame opcodes. Receive reports
 * the WinHTTP buffer type instead (0 = BINARY_MESSAGE, 2 = UTF8_MESSAGE,
 * 1/3 = binary/UTF8 fragment, 4 = CLOSE); ping/pong is handled internally
 * by WinHTTP and never reaches the application.
 */

#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <string.h>

/* Maximum bytes of each received message shown in the hex dump. */
#define HEXDUMP_MAX 64

/* Print a human-readable error for an explicit error code.
 * The WinHttpWebSocket* functions return the code directly instead of
 * setting GetLastError(). */
static void print_err_code(const char *step, DWORD err)
{
    char msg[512] = {0};

    DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    HMODULE hWinhttp = NULL;
    if (err >= 12000) {                 /* WINHTTP_ERROR_BASE */
        hWinhttp = GetModuleHandleW(L"winhttp.dll");
        if (hWinhttp)
            flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    DWORD len = FormatMessageA(flags, hWinhttp, err, 0,
                               msg, sizeof(msg), NULL);
    while (len > 0 && (msg[len-1] == '\n' || msg[len-1] == '\r' ||
                       msg[len-1] == ' '))
        msg[--len] = '\0';

    if (len > 0)
        fprintf(stderr, "[!] %s: error %lu - %s\n", step, err, msg);
    else
        fprintf(stderr, "[!] %s: error %lu\n", step, err);
}

/* Map a WinHTTP WebSocket buffer type to a printable name. */
static const char *buffer_type_name(WINHTTP_WEB_SOCKET_BUFFER_TYPE type)
{
    switch (type) {
    case WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE:  return "BINARY_MESSAGE";
    case WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE: return "BINARY_FRAGMENT";
    case WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE:    return "UTF8_MESSAGE";
    case WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE:   return "UTF8_FRAGMENT";
    case WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE:           return "CLOSE";
    default:                                             return "UNKNOWN";
    }
}

/* Print a buffer as a classic hex dump: 16 bytes per line, address, hex
 * values, ASCII column. Used to inspect raw server messages of unknown
 * format. */
static void hex_dump(const unsigned char *data, DWORD len)
{
    for (DWORD row = 0; row < len; row += 16) {
        printf("    %04lx  ", row);
        for (DWORD i = 0; i < 16; i++) {
            if (row + i < len)
                printf("%02x ", data[row + i]);
            else
                printf("   ");
            if (i == 7)
                putchar(' ');
        }
        printf(" |");
        for (DWORD i = 0; i < 16 && row + i < len; i++) {
            unsigned char c = data[row + i];
            putchar((c >= 0x20 && c < 0x7f) ? c : '.');
        }
        printf("|\n");
    }
}

/* Print one complete received message: header line, hex dump of the first
 * HEXDUMP_MAX bytes, printable text form. The message does not need to be
 * NUL-terminated - the length is explicit. */
static void print_message(int index, WINHTTP_WEB_SOCKET_BUFFER_TYPE type,
                          const char *msg, DWORD msg_len)
{
    printf("[%d] Received: type=%d (%s), len=%lu\n",
           index, (int)type, buffer_type_name(type), msg_len);

    if (msg_len > 0) {
        DWORD dump_len = (msg_len < HEXDUMP_MAX) ? msg_len : HEXDUMP_MAX;
        hex_dump((const unsigned char *)msg, dump_len);
        if (msg_len > dump_len)
            printf("    ... (%lu more bytes not dumped)\n", msg_len - dump_len);

        printf("    text: \"");
        for (DWORD i = 0; i < msg_len; i++) {
            unsigned char c = (unsigned char)msg[i];
            putchar((c >= 0x20 && c < 0x7f) ? c : '.');
        }
        printf("\"\n");

        /* The assignment expects the first server command to be "hello" -
         * flag it if the payload mentions it. */
        for (DWORD i = 0; i + 5 <= msg_len; i++) {
            if (memcmp(msg + i, "hello", 5) == 0) {
                printf("[+] first command: hello\n");
                break;
            }
        }
    }
    fflush(stdout);                     /* show the message in the terminal at once */
}

int main(int argc, char *argv[])
{
    /* ----- Stage 1: the URL must come from the command line ----- */
    if (argc != 2) {
        fprintf(stderr,
                "usage: relay_client.exe <URL>\n"
                "example: relay_client.exe https://relay.example.com/agent\n");
        return 1;
    }

    /* Convert the URL to UTF-16 (WinHTTP functions take wide strings). */
    wchar_t url[2048];
    if (MultiByteToWideChar(CP_ACP, 0, argv[1], -1, url, 2048) == 0) {
        print_err_code("MultiByteToWideChar(URL)", GetLastError());
        return 1;
    }

    /* Resource state for correct cleanup via goto. */
    int       rc = 1;                  /* assume failure by default */
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    HINTERNET hWebSocket = NULL;

    /* ----- Stage 2: split the URL into components (WinHttpCrackUrl) ----- */
    URL_COMPONENTS uc;
    ZeroMemory(&uc, sizeof(uc));
    uc.dwStructSize = sizeof(uc);

    wchar_t host[256];
    wchar_t path[2048];
    uc.lpszHostName    = host;  uc.dwHostNameLength = 256;
    uc.lpszUrlPath     = path;  uc.dwUrlPathLength  = 2048;
    /* lpszScheme / nScheme / nPort are filled by WinHttpCrackUrl. */

    if (!WinHttpCrackUrl(url, 0, 0, &uc)) {
        print_err_code("WinHttpCrackUrl (invalid URL?)", GetLastError());
        goto cleanup;
    }
    /* WinHttpConnect/OpenRequest below speak http(s); accept both, and let
     * the SECURE flag turn https into TLS. Anything else (ftp, file, ...)
     * cannot be handled here. */
    if (uc.nScheme != INTERNET_SCHEME_HTTP &&
        uc.nScheme != INTERNET_SCHEME_HTTPS) {
        fprintf(stderr, "[!] only http:// and https:// URLs are supported\n");
        goto cleanup;
    }
    if (uc.dwHostNameLength == 0) {
        fprintf(stderr, "[!] URL has no host name\n");
        goto cleanup;
    }
    BOOL https = (uc.nScheme == INTERNET_SCHEME_HTTPS);

    /* ----- Stage 3: session -> connection -> upgrade request ----- */
    printf("[1] Connecting to %ls://%ls%ls ... ",
           https ? L"https" : L"http", uc.lpszHostName, uc.lpszUrlPath);
    fflush(stdout);

    hSession = WinHttpOpen(L"relay_client/1.0",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { print_err_code("WinHttpOpen", GetLastError()); goto cleanup; }

    hConnect = WinHttpConnect(hSession, uc.lpszHostName, uc.nPort, 0);
    if (!hConnect) { print_err_code("WinHttpConnect", GetLastError()); goto cleanup; }

    /* An ordinary HTTPS GET - the WebSocket headers are added by the
     * UPGRADE option below, not by us. */
    DWORD req_flags = WINHTTP_FLAG_REFRESH;
    if (https) req_flags |= WINHTTP_FLAG_SECURE;   /* enables TLS for https */
    hRequest = WinHttpOpenRequest(hConnect, L"GET", uc.lpszUrlPath, NULL,
                                  WINHTTP_NO_REFERER,
                                  WINHTTP_DEFAULT_ACCEPT_TYPES,
                                  req_flags);
    if (!hRequest) { print_err_code("WinHttpOpenRequest", GetLastError()); goto cleanup; }

    /* This option takes no buffer: lpBuffer must be NULL and length 0. */
    if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
                          NULL, 0)) {
        print_err_code("WinHttpSetOption(UPGRADE_TO_WEB_SOCKET)", GetLastError());
        goto cleanup;
    }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        print_err_code("WinHttpSendRequest", GetLastError()); goto cleanup;
    }

    /* Required step: receive the handshake response ("101 Switching
     * Protocols") before completing the upgrade. */
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        print_err_code("WinHttpReceiveResponse", GetLastError()); goto cleanup;
    }

    /* Finish the handshake. On success the server answered
     * "101 Switching Protocols" and we get a WebSocket handle. */
    hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, 0);
    if (!hWebSocket) {
        print_err_code("WinHttpWebSocketCompleteUpgrade", GetLastError());
        goto cleanup;
    }
    WinHttpCloseHandle(hRequest);       /* the request handle is spent */
    hRequest = NULL;
    printf("connected (HTTP 101 Switching Protocols)\n");

    /* ----- Stage 4: receive-first command loop. The relay sends the first
     * message itself, so nothing is ever sent by this client: it listens
     * and prints every complete message until the server closes or an
     * error occurs. ----- */
    printf("[2] Listening for server messages (nothing is sent)...\n");
    for (int index = 1; ; index++) {
        char  buf[4096];                /* raw receive buffer */
        char  msg[8192];                /* accumulated complete message */
        DWORD msg_len = 0;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE msg_type =
            WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE;

        /* A message may arrive in parts: fragments first, and the final
         * piece is marked as *_MESSAGE - that is when it is complete. */
        for (;;) {
            DWORD got = 0;
            WINHTTP_WEB_SOCKET_BUFFER_TYPE type;
            DWORD werr = WinHttpWebSocketReceive(hWebSocket, buf, sizeof(buf),
                                                 &got, &type);
            if (werr != NO_ERROR) {
                print_err_code("WinHttpWebSocketReceive", werr);
                goto cleanup;
            }

            /* The server may close instead of sending anything. */
            if (type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE) {
                USHORT status = 0;
                char   reason[128];
                DWORD  reason_len = 0;
                if (WinHttpWebSocketQueryCloseStatus(hWebSocket, &status,
                                                     reason, sizeof(reason) - 1,
                                                     &reason_len) == NO_ERROR)
                    fprintf(stderr,
                            "[!] Server closed the connection (status %u).\n",
                            status);
                else
                    fprintf(stderr, "[!] Server closed the connection.\n");
                goto close;
            }

            if (got > 0 && msg_len + got < sizeof(msg)) {
                memcpy(msg + msg_len, buf, got);
                msg_len += got;
                msg_type = type;
            }
            if (type == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE ||
                type == WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE) {
                msg_type = type;        /* complete message received */
                break;
            }
        }

        print_message(index, msg_type, msg, msg_len);
    }

close:
    /* ----- Stage 5: close the connection cleanly ----- */
    {
        DWORD werr = WinHttpWebSocketShutdown(hWebSocket,
                                              WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
                                              NULL, 0);
        if (werr != NO_ERROR) {
            print_err_code("WinHttpWebSocketShutdown", werr);
        } else {
            USHORT status = 0;
            char   reason[128];
            DWORD  reason_len = 0;
            if (WinHttpWebSocketQueryCloseStatus(hWebSocket, &status,
                                                 reason, sizeof(reason) - 1,
                                                 &reason_len) == NO_ERROR)
                printf("[3] Connection closed cleanly (close status %u).\n", status);
            else
                printf("[3] Close frame sent.\n");
        }
    }
    rc = 0;

cleanup:
    if (hWebSocket) WinHttpCloseHandle(hWebSocket);
    if (hRequest)   WinHttpCloseHandle(hRequest);
    if (hConnect)   WinHttpCloseHandle(hConnect);
    if (hSession)   WinHttpCloseHandle(hSession);
    return rc;
}
