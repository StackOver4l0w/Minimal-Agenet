/* relay_client - a minimal information-only agent for Windows (WinHTTP, 8+).
 *
 * A "minimal agent" in the relay protocol is anything that answers the
 * operator panel's Hello command with a 750-byte identity frame. Everything
 * else (file browsing, shells, screenshots) is optional and advertised - or
 * deliberately not advertised - via the capability mask. This agent
 * advertises nothing: the panel shows its device information and offers no
 * remote-control UI. An honest, information-only agent.
 *
 * Lifecycle:
 *   [1] connect: session -> connection -> HTTP GET -> 101 upgrade -> socket
 *   [2] serve:   wait for a panel command, dispatch it, send one reply
 *   [3] shutdown: close the WebSocket cleanly and release handles
 *
 * Build (MinGW gcc):
 *   gcc -O2 -s -Wall -Wextra -o relay_client.exe relay_client.c -lwinhttp -ladvapi32
 * Run:
 *   relay_client.exe <URL>        e.g. ... https://relay.example.com/agent
 *
 * The URL comes only from the command line; nothing is hardcoded.
 */

/*
 * ============================================================================
 * Protocol reference (what the panel expects, byte for byte)
 * ============================================================================
 *
 * Every operator command:  [1-byte opcode][command-specific payload]
 * Every agent reply:       [UINT32 LE status][reply-specific payload]
 *                          status 0 = success, non-zero = error
 *
 * Opcodes this agent will ever see:
 *   0x00 Hello - "identify yourself". Reply: identity frame (see below).
 *   0x09 Exit  - terminate the agent. The ONLY command with no reply.
 *   everything else is not implemented: reply status 1.
 *
 * The identity frame (750 bytes, all strings ASCII NUL-padded):
 *
 *   offset  size  field
 *   ------  ----  -------------------------------------------------
 *   0       4     status = 0
 *   4       16    machine UUID, .NET Guid byte order (see get_machine_uuid)
 *   20      256   hostname
 *   276     256   logged-on user name
 *   532     32    CPU architecture ("x64" / "x86")
 *   564     32    platform ("Windows")
 *   596     128   OS version ("10.0.19045")
 *   724     4     build number (display only)
 *   728     9     commit hash (8 chars + NUL, display only)
 *   737     4     API version = 4
 *   741     1     is-64-bit-process flag
 *   742     8     capability mask (8 zero bytes = nothing implemented)
 *
 * Capability mask: 8 bytes, one bit per feature category, LSB first:
 *   bit 0 = FileSystem (ListDirectory/ReadFile/HashFile)
 *   bit 1 = Shell      (Open/Write/Read/CloseShell)
 *   bit 2 = Display    (GetDisplays/GetScreenshot)
 * An all-zero mask is honored literally: the panel hides all remote-control
 * UI and shows only this agent's identity fields.
 */

#include <windows.h>
#include <winhttp.h>
#include <stdio.h>

/* ==========================================================================
 * Protocol constants
 * ======================================================================== */

/* Command opcodes (full table; incoming requests are decoded with it). */
#define CMD_HELLO               0x00
#define CMD_LIST_DIRECTORY      0x01
#define CMD_READ_FILE           0x02
#define CMD_HASH_FILE           0x03
#define CMD_WRITE_SHELL         0x04
#define CMD_READ_SHELL          0x05
#define CMD_GET_DISPLAYS        0x06
#define CMD_GET_SCREENSHOT      0x07
#define CMD_CLOSE_SHELL         0x08
#define CMD_EXIT                0x09
#define CMD_OPEN_SHELL          0x0A

/* Reply status codes. */
#define STATUS_OK               0
#define STATUS_ERROR            1

/* Identity ("Hello") frame - field widths from the protocol table above. */
#define IDENTITY_FRAME_SIZE     750
#define ID_HOSTNAME_SIZE        256
#define ID_USERNAME_SIZE        256
#define ID_ARCH_SIZE            32
#define ID_PLATFORM_SIZE        32
#define ID_OS_VERSION_SIZE      128
#define ID_COMMIT_HASH_SIZE     9      /* 8 chars + NUL            */
#define ID_API_VERSION          4      /* v4 = current framing     */
#define ID_BUILD_NUMBER         1      /* display-only build tag   */

/* Capability mask: zero = information-only agent, no remote control. */
#define CAPABILITY_MASK         0

/* ==========================================================================
 * Buffers and output limits
 * ======================================================================== */

/* One WinHttpWebSocketReceive call returns ONE fragment; 64 KB matches what
 * real agents use (a shorter buffer would fail on large fragments). */
#define RECV_FRAGMENT_SIZE      65536

/* Commands are short; anything longer is kept up to this size and flagged. */
#define MAX_MESSAGE_SIZE        8192

/* Bytes of each message shown in the hex dump (keeps output readable). */
#define HEXDUMP_LIMIT           64

/* ==========================================================================
 * Diagnostics
 * ======================================================================== */

/* ---------------------------------------------------------------------------
 * print_error_code - human-readable text for an explicit WinAPI error code.
 *
 * Why an explicit code parameter? The WinHttpWebSocket* family returns the
 * error code directly (DWORD), unlike classic WinHTTP which returns BOOL and
 * leaves the code in GetLastError(). One helper serves both families.
 * ------------------------------------------------------------------------- */
static void print_error_code(const char *step, DWORD err)
{
    char msg[512] = {0};

    /* System codes come from the OS table; WinHTTP codes (>= 12000) live in
     * winhttp.dll's own message table. */
    DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    HMODULE winhttp = NULL;
    if (err >= 12000) {                 /* WINHTTP_ERROR_BASE */
        winhttp = GetModuleHandleW(L"winhttp.dll");
        if (winhttp)
            flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    DWORD len = FormatMessageA(flags, winhttp, err, 0, msg, sizeof(msg), NULL);
    while (len > 0 && (msg[len-1] == '\n' || msg[len-1] == '\r' ||
                       msg[len-1] == ' '))
        msg[--len] = '\0';

    if (len > 0)
        fprintf(stderr, "[!] %s: error %lu - %s\n", step, err, msg);
    else
        fprintf(stderr, "[!] %s: error %lu\n", step, err);
}

/* ---------------------------------------------------------------------------
 * ws_buffer_type_name - WinHTTP's WebSocket buffer type, as a string.
 * WinHTTP does not expose RFC 6455 frame opcodes; this enum is the only
 * "type" the application ever sees (ping/pong is handled internally).
 * ------------------------------------------------------------------------- */
static const char *ws_buffer_type_name(WINHTTP_WEB_SOCKET_BUFFER_TYPE type)
{
    switch (type) {
    case WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE:  return "binary-message";
    case WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE: return "binary-fragment";
    case WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE:    return "utf8-message";
    case WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE:   return "utf8-fragment";
    case WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE:           return "close";
    default:                                             return "unknown";
    }
}

/* ---------------------------------------------------------------------------
 * command_name - the relay-protocol name of an incoming command opcode.
 * ------------------------------------------------------------------------- */
static const char *command_name(unsigned char opcode)
{
    switch (opcode) {
    case CMD_HELLO:            return "Hello";
    case CMD_LIST_DIRECTORY:   return "ListDirectory";
    case CMD_READ_FILE:        return "ReadFile";
    case CMD_HASH_FILE:        return "HashFile";
    case CMD_WRITE_SHELL:      return "WriteShell";
    case CMD_READ_SHELL:       return "ReadShell";
    case CMD_GET_DISPLAYS:     return "GetDisplays";
    case CMD_GET_SCREENSHOT:   return "GetScreenshot";
    case CMD_CLOSE_SHELL:      return "CloseShell";
    case CMD_EXIT:             return "Exit";
    case CMD_OPEN_SHELL:       return "OpenShell";
    default:                   return "unknown";
    }
}

/* ==========================================================================
 * Wire-format writers (everything on the wire is little-endian)
 * ======================================================================== */

static void write_u32_le(unsigned char *buf, int *pos, unsigned value)
{
    for (int i = 0; i < 4; i++)
        buf[(*pos)++] = (unsigned char)(value >> (8 * i));
}

static void write_u64_le(unsigned char *buf, int *pos,
                         unsigned long long value)
{
    for (int i = 0; i < 8; i++)
        buf[(*pos)++] = (unsigned char)(value >> (8 * i));
}

/* Write an ASCII string into a fixed-width, NUL-padded protocol field.
 * The frame is pre-zeroed by the caller, so only string + NUL are written
 * and the remaining bytes of the field stay zero. */
static void write_ascii_field(unsigned char *buf, int *pos,
                              const char *s, int width)
{
    int start = *pos;
    int i = 0;
    while (s[i] != '\0' && i < width - 1) {
        buf[start + i] = (unsigned char)s[i];
        i++;
    }
    buf[start + i] = '\0';
    *pos = start + width;
}

/* ==========================================================================
 * System facts (what this agent reports about its machine)
 * ======================================================================== */

/* Get the machine UUID from HKLM\...\Cryptography\MachineGuid, converted to
 * the .NET Guid byte order the panel expects. Falls back to all zeros.
 *
 * The registry stores the UUID as text ("00112233-4455-6677-..."); the
 * panel parses the 16 frame bytes as a .NET Guid, whose first three groups
 * (Data1..Data3) are little-endian and whose last group is raw. Hence the
 * reorder at the end: string order -> 33 22 11 00 | 55 44 | 77 66 | raw. */
static void get_machine_uuid(unsigned char out[16])
{
    char text[64] = {0};
    DWORD size = sizeof(text) - 1;

    HKEY key;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography",
                      0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS) {
        DWORD type = 0;
        if (RegQueryValueExA(key, "MachineGuid", NULL, &type,
                             (LPBYTE)text, &size) != ERROR_SUCCESS ||
            type != REG_SZ)
            text[0] = '\0';
        RegCloseKey(key);
    }

    /* Hex digits (skipping '-', '{', '}') -> 16 bytes in string order. */
    unsigned char straight[16];
    int digits = 0;
    for (const char *p = text; *p != '\0' && digits < 32; p++) {
        int v;
        if (*p >= '0' && *p <= '9')      v = *p - '0';
        else if (*p >= 'a' && *p <= 'f') v = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'F') v = *p - 'A' + 10;
        else continue;
        straight[digits / 2] = (unsigned char)((digits % 2 == 0)
                                ? (v << 4) : (straight[digits / 2] | v));
        digits++;
    }
    if (digits != 32) {
        ZeroMemory(out, 16);             /* malformed or missing -> zeros */
        return;
    }

    /* String byte order -> .NET Guid layout (Data1..3 LE, Data4 raw). */
    out[0] = straight[3];  out[1] = straight[2];
    out[2] = straight[1];  out[3] = straight[0];
    out[4] = straight[5];  out[5] = straight[4];
    out[6] = straight[7];  out[7] = straight[6];
    for (int i = 0; i < 8; i++)
        out[8 + i] = straight[8 + i];
}

/* Everything the identity frame needs to know about the machine. */
typedef struct {
    char hostname[ID_HOSTNAME_SIZE];
    char username[ID_USERNAME_SIZE];
    char os_version[ID_OS_VERSION_SIZE];
} system_facts;

static void collect_system_facts(system_facts *facts)
{
    ZeroMemory(facts, sizeof(*facts));

    DWORD n = sizeof(facts->hostname);
    GetComputerNameA(facts->hostname, &n);       /* kernel32 */

    n = sizeof(facts->username);
    GetUserNameA(facts->username, &n);           /* advapi32 */

    /* RtlGetVersion (ntdll) reports the true OS version; the classic
     * GetVersionEx lies to apps without a compatibility manifest. */
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (ntdll) {
        /* GetProcAddress cast: the documented idiom for obtaining a typed
         * function pointer; -Wcast-function-type cannot verify signatures,
         * so it is suppressed for exactly this cast. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
        LONG (WINAPI *rtl_get_version)(LPOSVERSIONINFOW) =
            (LONG (WINAPI *)(LPOSVERSIONINFOW))
                GetProcAddress(ntdll, "RtlGetVersion");
#pragma GCC diagnostic pop
        if (rtl_get_version) {
            OSVERSIONINFOW info;
            ZeroMemory(&info, sizeof(info));
            info.dwOSVersionInfoSize = sizeof(info);
            if (rtl_get_version(&info) == 0)
                snprintf(facts->os_version, sizeof(facts->os_version),
                         "%lu.%lu.%lu", info.dwMajorVersion,
                         info.dwMinorVersion, info.dwBuildNumber);
        }
    }
}

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
    write_ascii_field(frame, &pos, "course01",       ID_COMMIT_HASH_SIZE);
    write_u32_le(frame, &pos, ID_API_VERSION);       /* API version = 4  */

    frame[pos++] = (unsigned char)(sizeof(void *) == 8 ? 1 : 0);  /* 64-bit */

    write_u64_le(frame, &pos, CAPABILITY_MASK);      /* capability mask  */

    return pos;    /* == IDENTITY_FRAME_SIZE */
}

/* Print the outgoing identity frame field by field (the mirror of
 * print_command: everything WE send is shown too, not just what we get). */
static void hex_dump(const unsigned char *data, DWORD length);

static void print_identity_frame(const unsigned char frame[IDENTITY_FRAME_SIZE],
                                 int frame_len)
{
    /* Numeric fields, little-endian. */
    unsigned status = frame[0]  | (frame[1]  << 8) |
                      (frame[2] << 16) | ((unsigned)frame[3]  << 24);
    unsigned build  = frame[724]| (frame[725] << 8) |
                      (frame[726]<< 16) | ((unsigned)frame[727] << 24);
    unsigned api    = frame[737]| (frame[738] << 8) |
                      (frame[739]<< 16) | ((unsigned)frame[740] << 24);
    unsigned is64   = frame[741];

    printf("[<] Identity frame (%d bytes):\n", frame_len);
    printf("    status  = %u\n", status);
    printf("    uuid    = ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", frame[4 + i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) putchar('-');
    }
    printf("  (machine, .NET Guid order)\n");
    printf("    host    = \"%s\"\n", (const char *)(frame + 20));
    printf("    user    = \"%s\"\n", (const char *)(frame + 276));
    printf("    arch    = \"%s\"\n", (const char *)(frame + 532));
    printf("    platform= \"%s\"\n", (const char *)(frame + 564));
    printf("    os      = \"%s\"\n", (const char *)(frame + 596));
    printf("    build   = %u, commit = \"%s\", api = %u, 64-bit = %u\n",
           build, (const char *)(frame + 728), api, is64);
    printf("    mask    = ");
    for (int i = 0; i < 8; i++)
        printf("%02x ", frame[742 + i]);
    printf(" (0 = information-only)\n");

    DWORD dump_len = (frame_len < HEXDUMP_LIMIT) ? (DWORD)frame_len
                                                 : HEXDUMP_LIMIT;
    hex_dump(frame, dump_len);
    if ((DWORD)frame_len > dump_len)
        printf("    ... (%d more bytes not dumped)\n", frame_len - (int)dump_len);

    fflush(stdout);
}

/* ==========================================================================
 * Transport: send one reply / receive one command
 * ======================================================================== */

/* Send one binary reply. Returns the WinHTTP error code (0 = success). */
static DWORD ws_send(HINTERNET socket, const void *data, DWORD length)
{
    return WinHttpWebSocketSend(socket,
                                WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
                                (PVOID)data, length);
}

/* One assembled incoming message. */
typedef struct {
    unsigned char data[MAX_MESSAGE_SIZE];
    DWORD length;
    BOOL truncated;               /* message exceeded MAX_MESSAGE_SIZE */
    WINHTTP_WEB_SOCKET_BUFFER_TYPE type;
} incoming_message;

/* ---------------------------------------------------------------------------
 * ws_receive - receive and assemble one complete WebSocket message.
 *
 * One WinHttpWebSocketReceive call yields one fragment; a message may span
 * several, and the last fragment carries the *_MESSAGE type. Returns the
 * WinHTTP error code (0 = success). If the server sent a close frame, the
 * return is 0 and *closed is set; the caller decides what to do.
 * ------------------------------------------------------------------------- */
static DWORD ws_receive(HINTERNET socket, incoming_message *msg, BOOL *closed)
{
    ZeroMemory(msg, sizeof(*msg));
    *closed = FALSE;

    for (;;) {
        unsigned char fragment[RECV_FRAGMENT_SIZE];
        DWORD got = 0;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE type;

        DWORD err = WinHttpWebSocketReceive(socket, fragment,
                                            sizeof(fragment), &got, &type);
        if (err != NO_ERROR)
            return err;

        if (type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE) {
            *closed = TRUE;
            return NO_ERROR;
        }

        if (got > 0) {
            if (msg->length + got <= MAX_MESSAGE_SIZE) {
                CopyMemory(msg->data + msg->length, fragment, got);
                msg->length += got;
            } else {
                msg->truncated = TRUE;   /* keep the first MAX_MESSAGE_SIZE */
            }
        }

        if (type == WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE ||
            type == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE) {
            msg->type = type;
            return NO_ERROR;             /* message complete */
        }
    }
}

/* ==========================================================================
 * Printing an incoming command (for the human watching the terminal)
 * ======================================================================== */

/* Classic 16-bytes-per-line hex dump with an ASCII column. */
static void hex_dump(const unsigned char *data, DWORD length)
{
    for (DWORD row = 0; row < length; row += 16) {
        printf("    %04lx  ", row);
        for (DWORD i = 0; i < 16; i++) {
            if (row + i < length)
                printf("%02x ", data[row + i]);
            else
                printf("   ");
            if (i == 7)
                putchar(' ');
        }
        printf(" |");
        for (DWORD i = 0; i < 16 && row + i < length; i++) {
            unsigned char c = data[row + i];
            putchar((c >= 0x20 && c < 0x7f) ? c : '.');
        }
        printf("|\n");
    }
}

/* Decode the command-specific part (everything after the 1-byte opcode) so
 * the terminal says WHAT the panel asked for, not just raw bytes. */
static void describe_command_payload(unsigned char opcode,
                                     const unsigned char *p, DWORD len)
{
    if (len == 0) {
        printf("    payload: (empty)\n");
        return;
    }

    switch (opcode) {
    case CMD_LIST_DIRECTORY:            /* UTF-16LE path + NUL */
    case CMD_HASH_FILE: {
        printf("    payload: path = \"");
        for (DWORD i = 0; i + 1 < len; i += 2) {
            unsigned char lo = p[i], hi = (i + 1 < len) ? p[i+1] : 0;
            unsigned short wc = (unsigned short)(lo | (hi << 8));
            if (wc == 0) break;         /* NUL terminator */
            putchar((wc >= 0x20 && wc < 0x7f) ? wc : '.');
        }
        printf("\"\n");
        break;
    }
    case CMD_READ_FILE: {               /* u64 size, u64 offset, then path */
        if (len >= 16) {
            unsigned long long size = 0, offset = 0;
            for (int i = 7; i >= 0; i--)   size   = (size   << 8) | p[i];
            for (int i = 15; i >= 8; i--) offset = (offset << 8) | p[i];
            printf("    payload: size = %llu, offset = %llu, path = \"",
                   size, offset);
            for (DWORD i = 16; i + 1 < len; i += 2) {
                unsigned short wc = (unsigned short)(p[i] | (p[i+1] << 8));
                if (wc == 0) break;
                putchar((wc >= 0x20 && wc < 0x7f) ? wc : '.');
            }
            printf("\"\n");
        } else {
            printf("    payload: (too short for ReadFile)\n");
        }
        break;
    }
    case CMD_GET_SCREENSHOT: {          /* u32 display, u32 quality, u32 full */
        if (len >= 12) {
            printf("    payload: display = %u, quality = %u, fullscreen = %u\n",
                   (unsigned)(p[0] | (p[1] << 8) | (p[2] << 16) |
                              ((unsigned)p[3] << 24)),
                   (unsigned)(p[4] | (p[5] << 8) | (p[6] << 16) |
                              ((unsigned)p[7] << 24)),
                   (unsigned)(p[8] | (p[9] << 8) | (p[10] << 16) |
                              ((unsigned)p[11] << 24)));
        } else {
            printf("    payload: (too short for GetScreenshot)\n");
        }
        break;
    }
    default:
        printf("    payload: (%lu bytes, see hex dump above)\n", len);
        break;
    }
}

/* Print one received command: header, decoded opcode and payload, hex dump,
 * printable text. Explicit length - the payload is NOT a C string. */
static void print_command(int index, const incoming_message *msg)
{
    printf("[%d] Received: type=%d (%s), len=%lu%s\n",
           index, (int)msg->type, ws_buffer_type_name(msg->type), msg->length,
           msg->truncated ? " [TRUNCATED]" : "");

    if (msg->length == 0) {
        fflush(stdout);
        return;
    }

    unsigned char opcode = msg->data[0];
    printf("    command: 0x%02x - %s\n", opcode, command_name(opcode));
    describe_command_payload(opcode, msg->data + 1, msg->length - 1);

    DWORD dump_len = (msg->length < HEXDUMP_LIMIT) ? msg->length
                                                   : HEXDUMP_LIMIT;
    hex_dump(msg->data, dump_len);
    if (msg->length > dump_len)
        printf("    ... (%lu more bytes not dumped)\n", msg->length - dump_len);

    printf("    text: \"");
    for (DWORD i = 0; i < msg->length; i++) {
        unsigned char c = msg->data[i];
        putchar((c >= 0x20 && c < 0x7f) ? c : '.');
    }
    printf("\"\n");

    if (opcode == CMD_HELLO)
        printf("[+] panel asks: who are you? (Hello)\n");

    fflush(stdout);                     /* show the message at once */
}

/* ==========================================================================
 * main
 * ======================================================================== */

int main(int argc, char *argv[])
{
    /* ----- Stage 1: the URL must come from the command line ----- */
    if (argc != 2) {
        fprintf(stderr,
                "usage: relay_client.exe <URL>\n"
                "example: relay_client.exe https://relay.example.com/agent\n");
        return 1;
    }

    wchar_t url[2048];
    if (MultiByteToWideChar(CP_ACP, 0, argv[1], -1, url, 2048) == 0) {
        print_error_code("MultiByteToWideChar(URL)", GetLastError());
        return 1;
    }

    /* Resource state for correct cleanup via goto. */
    int       rc = 1;                   /* assume failure by default  */
    HINTERNET session = NULL, connection = NULL, request = NULL;
    HINTERNET socket = NULL;

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
        goto cleanup;
    }
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

    /* ----- Stage 4: serve commands (the minimal-agent contract).
     * Every request gets exactly one reply - except Exit, which gets none
     * and terminates the agent. ----- */
    printf("[2] Agent mode: replying to commands (capability mask = 0)...\n");
    for (int index = 1; ; index++) {
        incoming_message msg;
        BOOL closed = FALSE;

        DWORD err = ws_receive(socket, &msg, &closed);
        if (err != NO_ERROR) {
            print_error_code("WinHttpWebSocketReceive", err);
            goto cleanup;
        }
        if (closed) {
            fprintf(stderr, "[!] Server closed the connection.\n");
            goto shutdown;
        }

        print_command(index, &msg);

        unsigned char opcode = (msg.length > 0) ? msg.data[0] : 0xFF;

        if (opcode == CMD_EXIT) {       /* Exit: no reply, terminate now */
            printf("[!] Exit requested - terminating.\n");
            fflush(stdout);
            rc = 0;                     /* a valid command, not an error */
            goto cleanup;               /* spec: terminate immediately  */
        }

        if (opcode == CMD_HELLO && msg.length == 1) {
            system_facts facts;
            collect_system_facts(&facts);

            unsigned char frame[IDENTITY_FRAME_SIZE];
            int frame_len = build_identity_frame(frame, &facts);

            err = ws_send(socket, frame, (DWORD)frame_len);
            if (err != NO_ERROR) {
                print_error_code("WinHttpWebSocketSend(identity frame)", err);
                goto cleanup;
            }
            printf("[+] identity sent to the panel (%d bytes)\n", frame_len);
            print_identity_frame(frame, frame_len);
            fflush(stdout);
        } else {                        /* not implemented: status = 1 */
            unsigned char status_error[4] = {1, 0, 0, 0};
            err = ws_send(socket, status_error, sizeof(status_error));
            if (err != NO_ERROR) {
                print_error_code("WinHttpWebSocketSend(status)", err);
                goto cleanup;
            }
            printf("[i] command 0x%02x not implemented - replied status 1 "
                   "(capability mask is 0)\n", opcode);
            fflush(stdout);
        }
    }

shutdown:
    /* ----- Stage 5: close the WebSocket cleanly ----- */
    {
        DWORD err = WinHttpWebSocketShutdown(socket,
                        WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
        if (err != NO_ERROR) {
            print_error_code("WinHttpWebSocketShutdown", err);
        } else {
            USHORT status = 0;
            char reason[128];
            DWORD reason_len = 0;
            if (WinHttpWebSocketQueryCloseStatus(socket, &status, reason,
                                                 sizeof(reason) - 1,
                                                 &reason_len) == NO_ERROR)
                printf("[3] Connection closed cleanly (close status %u).\n",
                       status);
            else
                printf("[3] Close frame sent.\n");
        }
    }
    rc = 0;

cleanup:
    if (socket)     WinHttpCloseHandle(socket);
    if (request)    WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    if (session)    WinHttpCloseHandle(session);
    return rc;
}
