/* ws_echo - a minimal WebSocket echo client for Windows (WinHTTP, Windows 8+).
 *
 * It performs the three classic echo-server steps:
 *   [1] create a WebSocket client and connect to  wss://echo.websocket.org
 *   [2] send a text message
 *   [3] read the echo (the same message) sent back by the server
 * then closes the connection cleanly.
 *
 *   Build (MinGW gcc):
 *       gcc -O2 -s -Wall -Wextra -o ws_echo.exe ws_echo.c -lwinhttp
 *   Run:
 *       ws_echo.exe [message]     (a default message is used if omitted)
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
 */

#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <string.h>

#define ECHO_HOST  L"echo.websocket.org"        /* wss:// = WebSocket over TLS */
#define ECHO_PORT  INTERNET_DEFAULT_HTTPS_PORT  /* 443                          */
#define ECHO_PATH  L"/"

/* Print a human-readable error for an explicit error code.
 * The WinHttpWebSocket* functions return the code directly instead of
 * setting GetLastError(), so the downloader's helper cannot be reused. */
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

int main(int argc, char *argv[])
{
    /* The message to send; argv[1] if given, otherwise a default. */
    char send_buf[1024];
    snprintf(send_buf, sizeof(send_buf), "%s",
             (argc >= 2) ? argv[1] : "Hello from C WebSocket client!");

    /* Resource state for correct cleanup via goto. */
    int       rc = 1;                  /* assume failure by default */
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    HINTERNET hWebSocket = NULL;

    /* ----- [1] Connect: session -> connection -> upgrade request ----- */
    printf("[1] Connecting to wss://%ls%ls ... ", ECHO_HOST, ECHO_PATH);

    hSession = WinHttpOpen(L"ws_echo/1.0",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { print_err_code("WinHttpOpen", GetLastError()); goto cleanup; }

    hConnect = WinHttpConnect(hSession, ECHO_HOST, ECHO_PORT, 0);
    if (!hConnect) { print_err_code("WinHttpConnect", GetLastError()); goto cleanup; }

    /* An ordinary HTTPS GET - the WebSocket headers are added by the
     * UPGRADE option below, not by us. */
    hRequest = WinHttpOpenRequest(hConnect, L"GET", ECHO_PATH, NULL,
                                  WINHTTP_NO_REFERER,
                                  WINHTTP_DEFAULT_ACCEPT_TYPES,
                                  WINHTTP_FLAG_SECURE);
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

    /* ----- [2] Send a text message ----- */
    printf("[2] Sending: \"%s\"\n", send_buf);
    DWORD werr = WinHttpWebSocketSend(hWebSocket,
                                      WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
                                      send_buf, (DWORD)strlen(send_buf));
    if (werr != NO_ERROR) {
        print_err_code("WinHttpWebSocketSend", werr);
        goto cleanup;
    }

    /* ----- [3] Read frames until the echo of our message arrives.
     * This server sends a greeting banner ("Request served by ...")
     * first, so more than one complete message may have to be read. ----- */
    BOOL matched = FALSE;
    for (int seen = 0; seen < 16 && !matched; seen++) {
        char  buf[4096];                /* raw receive buffer */
        char  msg[8192];                /* accumulated message */
        DWORD msg_len = 0;

        /* A message may arrive in parts: fragments first, and the final
         * piece is marked as *_MESSAGE - that is when it is complete. */
        for (;;) {
            DWORD got = 0;
            WINHTTP_WEB_SOCKET_BUFFER_TYPE type;
            werr = WinHttpWebSocketReceive(hWebSocket, buf, sizeof(buf),
                                           &got, &type);
            if (werr != NO_ERROR) {
                print_err_code("WinHttpWebSocketReceive", werr);
                goto cleanup;
            }

            /* The server may close instead of echoing. */
            if (type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE) {
                fprintf(stderr, "[!] Server closed the connection.\n");
                goto cleanup;
            }

            if (got > 0 && msg_len + got < sizeof(msg)) {
                memcpy(msg + msg_len, buf, got);
                msg_len += got;
            }
            if (type == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE ||
                type == WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE)
                break;                  /* complete message received */
        }
        msg[msg_len] = '\0';

        printf("[3] Received: \"%s\"\n", msg);
        if (strcmp(msg, send_buf) == 0)
            matched = TRUE;
        else
            printf("    (server banner - keep reading for the echo...)\n");
    }
    if (matched)
        printf("    MATCH: the echoed message is identical - client works!\n");
    else
        printf("    MISMATCH: no echo of our message was received.\n");

    /* ----- [4] Close the connection cleanly ----- */
    werr = WinHttpWebSocketShutdown(hWebSocket,
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
            printf("[4] Connection closed cleanly (close status %u).\n", status);
        else
            printf("[4] Close frame sent.\n");
    }
    rc = 0;                            /* success */

cleanup:
    if (hWebSocket) WinHttpCloseHandle(hWebSocket);
    if (hRequest)   WinHttpCloseHandle(hRequest);
    if (hConnect)   WinHttpCloseHandle(hConnect);
    if (hSession)   WinHttpCloseHandle(hSession);
    return rc;
}
