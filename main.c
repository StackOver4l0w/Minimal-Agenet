/* downloader - downloads a file over HTTP(S) using WinHTTP.
 *
 *   Build (MinGW gcc):
 *       gcc -O2 -s -Wall -Wextra -o downloader.exe main.c -lwinhttp
 *   Run:
 *       downloader.exe <URL> <FILE>
 *   Example:
 *       downloader.exe https://httpbin.org/image/png out.png
 *
 * WinHTTP flow (layered handles: session -> connection -> request):
 *   WinHttpCrackUrl  -> split the URL into host/port/path/scheme
 *   WinHttpOpen      -> create a session (user-agent + system proxy)
 *   WinHttpConnect   -> connect to host:port
 *   WinHttpOpenRequest -> build a GET (SECURE flag enables TLS for https)
 *   WinHttpSendRequest + WinHttpReceiveResponse -> send and receive headers
 *   WinHttpQueryHeaders -> check HTTP status (200) and Content-Length
 *   loop: WinHttpQueryDataAvailable + WinHttpReadData -> write to file in chunks
 *   WinHttpCloseHandle x3 + fclose -> release resources
 */

#include <windows.h>
#include <winhttp.h>
#include <stdio.h>

/* Print a human-readable Windows/WinHTTP error for the given step.
 * Reads the code via GetLastError() and turns it into text via FormatMessage.
 * For WinHTTP codes (>= 12000) the text comes from the winhttp.dll module. */
static void print_error(const char *step)
{
    DWORD err = GetLastError();
    char  msg[512] = {0};

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
    /* ----- Stage 1: parse command-line arguments ----- */
    if (argc < 3) {
        fprintf(stderr,
            "Usage: %s <URL> <FILE>\n"
            "Example:      %s https://httpbin.org/image/png picture.png\n",
            argv[0], argv[0]);
        return 1;
    }

    const char *url_ansi  = argv[1];   /* the URL (narrow string) */
    const char *file_path = argv[2];   /* where to save the file */

    /* All WinHTTP functions take wide strings (wchar_t).
     * Convert the URL to wide (console code page, CP_ACP). */
    wchar_t url[2048];
    if (MultiByteToWideChar(CP_ACP, 0, url_ansi, -1, url, 2048) == 0) {
        print_error("MultiByteToWideChar(URL)");
        return 1;
    }

    /* Resource state for correct cleanup via goto. */
    int       rc = 1;                  /* assume failure by default */
    FILE     *fp = NULL;
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;

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
        print_error("WinHttpCrackUrl (invalid URL?)");
        goto cleanup;
    }
    BOOL https = (uc.nScheme == INTERNET_SCHEME_HTTPS);

    /* ----- Stage 3: session and connection ----- */
    hSession = WinHttpOpen(L"downloader/1.0",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { print_error("WinHttpOpen"); goto cleanup; }

    hConnect = WinHttpConnect(hSession, uc.lpszHostName, uc.nPort, 0);
    if (!hConnect) { print_error("WinHttpConnect"); goto cleanup; }

    /* ----- Stage 4: request, send, receive response ----- */
    DWORD req_flags = WINHTTP_FLAG_REFRESH;
    if (https) req_flags |= WINHTTP_FLAG_SECURE;   /* enables TLS for https */

    hRequest = WinHttpOpenRequest(hConnect, L"GET", uc.lpszUrlPath, NULL,
                                  WINHTTP_NO_REFERER,
                                  WINHTTP_DEFAULT_ACCEPT_TYPES, req_flags);
    if (!hRequest) { print_error("WinHttpOpenRequest"); goto cleanup; }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        print_error("WinHttpSendRequest"); goto cleanup;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        print_error("WinHttpReceiveResponse"); goto cleanup;
    }

    /* ----- Stage 5: check HTTP status + Content-Length ----- */
    DWORD status = 0, sz = sizeof(status);
    if (!WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz,
            WINHTTP_NO_HEADER_INDEX)) {
        print_error("WinHttpQueryHeaders(status)"); goto cleanup;
    }
    if (status != 200) {
        /* Don't create a file for an error page (404/500 etc.). */
        fprintf(stderr, "[!] Server returned HTTP %lu (expected 200).\n", status);
        goto cleanup;
    }

    DWORD content_len = 0; sz = sizeof(content_len);
    BOOL have_len = WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &content_len, &sz,
            WINHTTP_NO_HEADER_INDEX);

    printf("Downloading: %s  ->  %s\n", url_ansi, file_path);
    if (have_len)
        printf("Size:   %lu bytes (%.2f KB)\n",
               content_len, content_len / 1024.0);
    else
        printf("Size:   unknown (no Content-Length / streamed)\n");

    /* ----- Stage 6: read in chunks and write to file ----- */
    fp = fopen(file_path, "wb");       /* binary mode! otherwise bytes get corrupted */
    if (!fp) { print_error("fopen(create output file)"); goto cleanup; }

    BYTE   buf[8192];
    DWORD  total = 0;
    for (;;) {
        DWORD avail = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &avail)) {
            print_error("WinHttpQueryDataAvailable"); goto cleanup;
        }
        if (avail == 0)
            break;                     /* body finished */

        DWORD to_read = (avail < sizeof(buf)) ? avail : (DWORD)sizeof(buf);
        DWORD got = 0;
        if (!WinHttpReadData(hRequest, buf, to_read, &got)) {
            print_error("WinHttpReadData"); goto cleanup;
        }
        if (got == 0)
            break;

        if (fwrite(buf, 1, got, fp) != got) {
            print_error("fwrite(write to disk)"); goto cleanup;
        }
        total += got;

        if (have_len && content_len > 0)
            fprintf(stderr, "\r  ... %lu / %lu bytes (%lu%%)",
                    total, content_len, total * 100 / content_len);
        else
            fprintf(stderr, "\r  ... %lu bytes", total);
    }
    fprintf(stderr, "\nDone: saved %lu bytes to '%s'.\n", total, file_path);
    rc = 0;                            /* success */

cleanup:
    /* ----- Stage 7: release resources (any path, success or error) ----- */
    if (fp)      fclose(fp);
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
    return rc;
}
