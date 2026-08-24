/* relay_client - a minimal agent for Windows (WinHTTP, 8+).
 *
 * A "minimal agent" in the relay protocol is anything that answers the
 * operator panel's Hello command with a 750-byte identity frame. Everything
 * else (file browsing, shells, screenshots) is optional and advertised - or
 * deliberately not advertised - via the capability mask.
 *
 * This agent implements the Shell category (mask bit 1): the panel opens a
 * terminal window whose commands run in a local cmd.exe, and their output
 * streams back. One side effect of advertising Shell without FileSystem:
 * the panel's file manager falls back to its PowerShell-over-shell backend,
 * so a basic file browser comes for free.
 *
 * Lifecycle:
 *   [1] connect: session -> connection -> HTTP GET -> 101 upgrade -> socket
 *   [2] serve:   wait for a panel command, dispatch it, send one reply
 *   [3] redial:  on a lost connection (transport error or a close frame),
 *               wait 1..32 s and dial again - an agent never gives up.
 *               Only the Exit command (or killing the process) ends it;
 *               live shells survive a redial.
 *
 * Build (MinGW gcc):
 *   gcc -O2 -s -Wall -Wextra -o relay_client.exe main.c \
 *       transport.c shell.c report.c system_facts.c -lwinhttp -ladvapi32
 * Run:
 *   relay_client.exe <URL>        e.g. ... https://relay.example.com/agent
 *   relay_client.exe <URL> -v     verbose: dump every command's raw bytes
 *
 * The URL comes only from the command line; nothing is hardcoded.
 *
 * ============================================================================
 * Module map (one header = one topic)
 * ============================================================================
 *   main.c           this file - connect, dispatch, cleanup (main)
 *   protocol.h       opcodes, statuses, identity frame, capability mask
 *   wire.h           tiny little-endian writers (header-only)
 *   transport.h/.c   the WebSocket pipe: ws_send / ws_receive
 *   shell.h/.c       the cmd.exe pool: spawn / read / write / teardown
 *   report.h/.c      human-facing output: errors, hex dumps, decoders
 *   system_facts.h/.c  machine UUID + hostname/user/OS facts
 */

#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <string.h>      /* strcmp (the -v flag) */

#include "protocol.h"
#include "wire.h"
#include "transport.h"
#include "shell.h"
#include "report.h"
#include "system_facts.h"

/* Commit tag baked into the identity frame. CI overrides it with
 * -DAGENT_COMMIT_HASH="<8-char git hash>"; local builds keep the study tag. */
#ifndef AGENT_COMMIT_HASH
#define AGENT_COMMIT_HASH "course01"
#endif

/* ==========================================================================
 * The identity frame (the reply to Hello)
 * ======================================================================== */

/* Build the full 750-byte identity frame. Returns the frame size. */
static int build_identity_frame(unsigned char frame[IDENTITY_FRAME_SIZE],
                                const system_facts *facts)
{
    ZeroMemory(frame, IDENTITY_FRAME_SIZE);
    int pos = 0;

    write_u32_le(frame, &pos, STATUS_OK);            /* status = 0       */

    unsigned char uuid[16];
    get_machine_uuid(uuid);
    CopyMemory(frame + pos, uuid, 16);               /* machine UUID     */
    pos += 16;

    write_ascii_field(frame, &pos, facts->hostname,  ID_HOSTNAME_SIZE);
    write_ascii_field(frame, &pos, facts->username,  ID_USERNAME_SIZE);
    write_ascii_field(frame, &pos,
                      sizeof(void *) == 8 ? "x64" : "x86", ID_ARCH_SIZE);
    write_ascii_field(frame, &pos, "Windows",        ID_PLATFORM_SIZE);
    write_ascii_field(frame, &pos, facts->os_version, ID_OS_VERSION_SIZE);

    write_u32_le(frame, &pos, ID_BUILD_NUMBER);      /* build number     */
    write_ascii_field(frame, &pos, AGENT_COMMIT_HASH, ID_COMMIT_HASH_SIZE);
    write_u32_le(frame, &pos, ID_API_VERSION);       /* API version = 4  */

    frame[pos++] = (unsigned char)(sizeof(void *) == 8 ? 1 : 0);  /* 64-bit */

    write_u64_le(frame, &pos, CAPABILITY_MASK);      /* capability mask  */

    return pos;    /* == IDENTITY_FRAME_SIZE */
}

/* ==========================================================================
 * Command handlers (one reply per request, except Exit)
 * ======================================================================== */

/* Verbose mode (-v): dump every command's raw bytes. Default is one line
 * per event, like a release agent - the full wire dump is a study tool. */
static int verbose = 0;

/* Reply to Hello: collect facts, build the frame, send it, show it. */
static DWORD handle_hello(HINTERNET socket)
{
    system_facts facts;
    collect_system_facts(&facts);

    unsigned char frame[IDENTITY_FRAME_SIZE];
    int frame_len = build_identity_frame(frame, &facts);

    DWORD err = ws_send(socket, frame, (DWORD)frame_len);
    if (err != NO_ERROR)
        return err;
    printf("[+] identity sent to the panel (%d bytes)\n", frame_len);
    if (verbose)
        print_identity_frame(frame, frame_len);
    fflush(stdout);
    return NO_ERROR;
}

/* OpenShell: find a free pool slot (the slot index IS the shell id). */
static DWORD handle_open_shell(HINTERNET socket)
{
    int id = shell_open();

    DWORD err;
    if (id < 0) {
        unsigned char status_error[4] = {1, 0, 0, 0};
        err = ws_send(socket, status_error, sizeof(status_error));
        printf("[i] OpenShell failed - replied status 1\n");
    } else {
        /* Exactly 12 bytes: the panel reads the id only when the
         * reply is at least this long, else it assumes id 0. */
        unsigned char reply[12];
        int pos = 0;
        write_u32_le(reply, &pos, STATUS_OK);
        write_u64_le(reply, &pos, (unsigned long long)id);
        err = ws_send(socket, reply, sizeof(reply));
        printf("[+] shell %d opened (cmd.exe spawned) - id sent\n", id);
    }
    fflush(stdout);
    return err;
}

/* WriteShell: [shellId:8][UTF-8 input + NUL]. The panel already appends
 * "\n" to commands - appending another newline would execute every
 * command twice. */
static DWORD handle_write_shell(HINTERNET socket, const incoming_message *msg)
{
    unsigned long long id = 0;
    for (int i = 8; i >= 1; i--)
        id = (id << 8) | msg->data[i];

    shell_slot *slot = shell_lookup(id);
    int status = STATUS_ERROR;
    if (slot) {
        /* Strip trailing NUL(s); "\x03" passes through as Ctrl+C. */
        DWORD end = msg->length;
        while (end > 9 && msg->data[end - 1] == '\0')
            end--;
        if (end > 9 && shell_write(slot, msg->data + 9, end - 9) == 0)
            status = STATUS_OK;
    }

    unsigned char reply[4] = {0, 0, 0, 0};
    reply[0] = (unsigned char)status;
    DWORD err = ws_send(socket, reply, sizeof(reply));
    if (err == NO_ERROR) {
        printf("[%s] write to shell %llu - status %d\n",
               slot ? "+" : "!", id, status);
        fflush(stdout);
    }
    return err;
}

/* ReadShell: drain what the shell has buffered. An empty chunk is legal
 * (that is "idle"); a dead shell reports status 1. */
static DWORD handle_read_shell(HINTERNET socket, const incoming_message *msg)
{
    unsigned long long id = 0;
    for (int i = 8; i >= 1; i--)
        id = (id << 8) | msg->data[i];

    shell_slot *slot = shell_lookup(id);
    unsigned char status_error[4] = {1, 0, 0, 0};
    DWORD err;

    if (!slot) {
        err = ws_send(socket, status_error, sizeof(status_error));
        if (err == NO_ERROR)
            printf("[!] read shell %llu - unknown id, status 1\n", id);
    } else {
        static unsigned char chunk[4 + SHELL_READ_CHUNK + 1];
        DWORD got = 0;
        int r = shell_read(slot, chunk + 4, SHELL_READ_CHUNK, &got);

        if (r == SHELL_READ_DEAD) {
            err = ws_send(socket, status_error, sizeof(status_error));
            if (err == NO_ERROR)
                printf("[!] shell %llu exited - status 1, slot freed\n",
                       id);
        } else {
            /* [status:4][chunk][NUL] - the NUL terminator is part of the
             * v4 contract and is written explicitly (the panel strips
             * exactly one). An empty chunk = "idle". */
            int pos = 0;
            write_u32_le(chunk, &pos, STATUS_OK);
            chunk[4 + got] = '\0';
            err = ws_send(socket, chunk, 4 + got + 1);
            if (err == NO_ERROR) {
                if (r == SHELL_READ_IDLE)
                    printf("[i] read shell %llu - idle\n", id);
                else
                    printf("[+] read shell %llu - %lu byte(s)\n",
                           id, got);
            }
        }
    }
    fflush(stdout);
    return err;
}

/* CloseShell: "close and forget" - unknown ids still get status 0. */
static DWORD handle_close_shell(HINTERNET socket, const incoming_message *msg)
{
    unsigned long long id = 0;
    for (int i = 8; i >= 1; i--)
        id = (id << 8) | msg->data[i];

    shell_slot *slot = shell_lookup(id);
    if (slot) {
        shell_teardown(slot);
        printf("[+] shell %llu closed (cmd.exe terminated)\n", id);
    } else {
        printf("[i] close shell %llu - not open (still ok)\n", id);
    }

    unsigned char reply[4] = {0, 0, 0, 0};
    DWORD err = ws_send(socket, reply, sizeof(reply));
    if (err == NO_ERROR)
        fflush(stdout);
    return err;
}

/* ==========================================================================
 * main
 * ======================================================================== */

/* One full connect/serve/close session (defined below main). */
static int run_session(const wchar_t *url, int *long_lived);

int main(int argc, char *argv[])
{
    /* ----- Stage 1: the URL must come from the command line ----- */
    if (argc == 3 && strcmp(argv[2], "-v") == 0)
        verbose = 1;
    else if (argc != 2) {
        fprintf(stderr,
                "usage: relay_client.exe <URL> [-v]\n"
                "example: relay_client.exe https://relay.example.com/agent\n"
                "-v = verbose: dump every command's raw bytes\n");
        return 1;
    }

    wchar_t url[2048];
    if (MultiByteToWideChar(CP_ACP, 0, argv[1], -1, url, 2048) == 0) {
        print_error_code("MultiByteToWideChar(URL)", GetLastError());
        return 1;
    }

    /* ----- The agent loop: dial, serve, redial. A lost connection is a
     * normal event (the relay drops agent sockets when the paired operator
     * disconnects, deploys recycle the Durable Object, idle NATs time out);
     * each loss is answered with a fresh dial after a capped backoff. Only
     * Exit (RC_EXIT) or an unrecoverable local failure ends the process.
     * Live shells survive a redial - only the panel's view of them is new
     * ids after the panel re-opens. */
    int rc = RC_SESSION_LOST;
    while (rc == RC_SESSION_LOST) {
        int long_lived = 0;
        rc = run_session(url, &long_lived);
        if (rc == RC_SESSION_LOST) {
            static const int backoff_steps[] = { 1, 2, 4, 8, 16, 32 };
            static const int backoff_count =
                sizeof(backoff_steps) / sizeof(backoff_steps[0]);
            static int backoff_pos = 0;

            int wait_s = backoff_steps[backoff_pos];
            /* Grow toward the cap while dialing keeps failing, but reset
             * after a session that lived a while - one blip on a healthy
             * day should not wait half a minute on the next one. */
            if (long_lived)
                backoff_pos = 0;
            else if (backoff_pos + 1 < backoff_count)
                backoff_pos++;

            printf("[i] connection lost - redialing in %d s ...\n", wait_s);
            fflush(stdout);
            Sleep((DWORD)wait_s * 1000);
        }
    }
    return rc;
}

/* One full session: connect, serve until the connection dies or Exit
 * arrives, close cleanly. Returns RC_EXIT / RC_SESSION_LOST / RC_LOCAL_ERROR.
 * All handles are released on every path (no leaks across redials).
 * *long_lived is set when the session served at least one command - the
 * caller uses it to reset the redial backoff after a healthy session. */
static int run_session(const wchar_t *url, int *long_lived)
{
    /* Resource state for correct cleanup via goto. */
    int       rc = RC_SESSION_LOST;     /* default: dial again          */
    HINTERNET session = NULL, connection = NULL, request = NULL;
    HINTERNET socket = NULL;

    *long_lived = 0;                    /* set on the first served reply */

    /* ----- Stage 2: split the URL into components (WinHttpCrackUrl) ----- */
    URL_COMPONENTS uc;
    ZeroMemory(&uc, sizeof(uc));
    uc.dwStructSize = sizeof(uc);

    wchar_t host[256];
    wchar_t path[2048];
    uc.lpszHostName    = host;  uc.dwHostNameLength = 256;
    uc.lpszUrlPath     = path;  uc.dwUrlPathLength  = 2048;

    if (!WinHttpCrackUrl(url, 0, 0, &uc)) {
        print_error_code("WinHttpCrackUrl (invalid URL?)", GetLastError());
        rc = RC_LOCAL_ERROR;          /* a bad URL will not heal itself */
        goto cleanup;
    }
    if (uc.nScheme != INTERNET_SCHEME_HTTP &&
        uc.nScheme != INTERNET_SCHEME_HTTPS) {
        fprintf(stderr, "[!] only http:// and https:// URLs are supported\n");
        rc = RC_LOCAL_ERROR;
        goto cleanup;
    }
    if (uc.dwHostNameLength == 0) {
        fprintf(stderr, "[!] URL has no host name\n");
        rc = RC_LOCAL_ERROR;
        goto cleanup;
    }
    BOOL https = (uc.nScheme == INTERNET_SCHEME_HTTPS);

    /* ----- Stage 3: session -> connection -> upgrade request ----- */
    printf("[1] Connecting to %ls://%ls%ls ... ",
           https ? L"https" : L"http", uc.lpszHostName, uc.lpszUrlPath);
    fflush(stdout);

    session = WinHttpOpen(L"relay_client/1.0",
                          WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                          WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) { print_error_code("WinHttpOpen", GetLastError()); goto cleanup; }

    connection = WinHttpConnect(session, uc.lpszHostName, uc.nPort, 0);
    if (!connection) { print_error_code("WinHttpConnect", GetLastError()); goto cleanup; }

    /* An ordinary HTTPS GET - the WebSocket headers are added by the
     * UPGRADE option below, not by us. */
    DWORD request_flags = WINHTTP_FLAG_REFRESH;
    if (https) request_flags |= WINHTTP_FLAG_SECURE;   /* TLS for https */
    request = WinHttpOpenRequest(connection, L"GET", uc.lpszUrlPath, NULL,
                                 WINHTTP_NO_REFERER,
                                 WINHTTP_DEFAULT_ACCEPT_TYPES, request_flags);
    if (!request) { print_error_code("WinHttpOpenRequest", GetLastError()); goto cleanup; }

    /* This option takes no buffer: lpBuffer must be NULL and length 0. */
    if (!WinHttpSetOption(request, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
                          NULL, 0)) {
        print_error_code("WinHttpSetOption(UPGRADE_TO_WEB_SOCKET)",
                         GetLastError());
        goto cleanup;
    }

    if (!WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        print_error_code("WinHttpSendRequest", GetLastError()); goto cleanup;
    }

    /* Required step: receive the handshake response ("101 Switching
     * Protocols") before completing the upgrade. */
    if (!WinHttpReceiveResponse(request, NULL)) {
        print_error_code("WinHttpReceiveResponse", GetLastError()); goto cleanup;
    }

    /* Finish the handshake; the request handle is spent and gets closed. */
    socket = WinHttpWebSocketCompleteUpgrade(request, 0);
    if (!socket) {
        print_error_code("WinHttpWebSocketCompleteUpgrade", GetLastError());
        goto cleanup;
    }
    WinHttpCloseHandle(request);
    request = NULL;
    printf("connected (HTTP 101 Switching Protocols)\n");

    /* ----- Stage 4: serve commands. Every request gets exactly one reply
     * - except Exit, which gets none and terminates the agent. ----- */
    printf("[2] Agent mode: replying to commands (capability mask = Shell)...\n");
    for (int index = 1; ; index++) {
        *long_lived = 1;                /* a command arrived on this wire */
        incoming_message msg;
        BOOL closed = FALSE;

        DWORD err = ws_receive(socket, &msg, &closed);
        if (err != NO_ERROR) {
            print_error_code("WinHttpWebSocketReceive", err);
            goto cleanup;             /* rc stays SESSION_LOST -> redial */
        }
        if (closed) {
            fprintf(stderr,
                    "[!] Server closed the connection - redialing.\n");
            goto cleanup;             /* a close frame is also just a loss */
        }

        if (verbose)
            print_command(index, &msg);

        unsigned char opcode = (msg.length > 0) ? msg.data[0] : 0xFF;

        /* A message that overflowed the receive buffer is refused whole:
         * executing its head would run half a command. */
        if (msg.truncated) {
            unsigned char status_error[4] = {1, 0, 0, 0};
            err = ws_send(socket, status_error, sizeof(status_error));
            if (err == NO_ERROR) {
                printf("[!] message over %d bytes - refused, status 1\n",
                       MAX_MESSAGE_SIZE);
                fflush(stdout);
                continue;
            }
            print_error_code("WinHttpWebSocketSend(reply)", err);
            goto cleanup;
        }

        if (opcode == CMD_EXIT) {       /* Exit: no reply, terminate now */
            printf("[!] Exit requested - terminating.\n");
            fflush(stdout);
            rc = RC_EXIT;               /* a valid command, not an error */
            goto cleanup;               /* spec: terminate immediately  */
        }

        if (opcode == CMD_HELLO && msg.length == 1) {
            err = handle_hello(socket);
        } else if (opcode == CMD_OPEN_SHELL) {
            err = handle_open_shell(socket);
        } else if (opcode == CMD_WRITE_SHELL && msg.length >= 9) {
            err = handle_write_shell(socket, &msg);
        } else if (opcode == CMD_READ_SHELL && msg.length >= 9) {
            err = handle_read_shell(socket, &msg);
        } else if (opcode == CMD_CLOSE_SHELL && msg.length >= 9) {
            err = handle_close_shell(socket, &msg);
        } else {                        /* not implemented: status = 1 */
            unsigned char status_error[4] = {1, 0, 0, 0};
            err = ws_send(socket, status_error, sizeof(status_error));
            if (err == NO_ERROR) {
                printf("[i] command 0x%02x not implemented - replied status 1\n",
                       opcode);
                fflush(stdout);
            }
        }
        if (err != NO_ERROR) {
            print_error_code("WinHttpWebSocketSend(reply)", err);
            goto cleanup;
        }
    }

    /* Unreachable: the serve loop above only leaves via goto cleanup. */

cleanup:
    if (socket)     WinHttpCloseHandle(socket);
    if (request)    WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    if (session)    WinHttpCloseHandle(session);
    return rc;
}
