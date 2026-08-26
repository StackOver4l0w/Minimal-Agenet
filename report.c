/* report.c - everything printed for the human (see report.h).
 */

#include "report.h"

#include <winhttp.h>
#include <stdio.h>

/* ---------------------------------------------------------------------------
 * print_error_code - human-readable text for an explicit WinAPI error code
 * (contract note in report.h).
 * ------------------------------------------------------------------------- */
void print_error_code(const char *step, DWORD err)
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
 * ------------------------------------------------------------------------- */
const char *ws_buffer_type_name(WINHTTP_WEB_SOCKET_BUFFER_TYPE type)
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
const char *command_name(unsigned char opcode)
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

/* Classic 16-bytes-per-line hex dump with an ASCII column. */
void hex_dump(const unsigned char *data, DWORD length)
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
void describe_command_payload(unsigned char opcode,
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
    case CMD_OPEN_SHELL:
        printf("    payload: (empty)\n");
        break;
    case CMD_WRITE_SHELL: {             /* u64 shellId, then UTF-8 input + NUL */
        if (len >= 8) {
            unsigned long long id = 0;
            for (int i = 7; i >= 0; i--) id = (id << 8) | p[i];
            printf("    payload: shell = %llu, input = \"", id);
            for (DWORD i = 8; i < len && p[i] != '\0'; i++) {
                unsigned char c = p[i];
                putchar((c >= 0x20 && c < 0x7f) ? c : '.');
            }
            printf("\"\n");
        } else {
            printf("    payload: (too short for WriteShell)\n");
        }
        break;
    }
    case CMD_READ_SHELL:
    case CMD_CLOSE_SHELL: {             /* u64 shellId */
        if (len >= 8) {
            unsigned long long id = 0;
            for (int i = 7; i >= 0; i--) id = (id << 8) | p[i];
            printf("    payload: shell = %llu\n", id);
        } else {
            printf("    payload: (too short for %s)\n",
                   opcode == CMD_READ_SHELL ? "ReadShell" : "CloseShell");
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

/* Print one received command: header, decoded opcode and payload, hex
 * dump, printable text. Explicit length - the payload is NOT a C string. */
void print_command(int index, const incoming_message *msg)
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

/* Print the outgoing identity frame field by field (the mirror of
 * print_command: everything WE send is shown too, not just what we get). */
void print_identity_frame(const unsigned char frame[IDENTITY_FRAME_SIZE],
                          int frame_len)
{
    /* Numeric fields, little-endian (v5 layout, see protocol.h). */
    unsigned status = frame[0]  | (frame[1]  << 8) |
                      (frame[2] << 16) | ((unsigned)frame[3]  << 24);
    unsigned api    = frame[4]  | (frame[5]  << 8) |
                      (frame[6] << 16) | ((unsigned)frame[7]  << 24);
    unsigned breed  = frame[8]  | (frame[9]  << 8) |
                      (frame[10]<< 16) | ((unsigned)frame[11] << 24);
    unsigned build  = frame[21]| (frame[22] << 8) |
                      (frame[23]<< 16) | ((unsigned)frame[24] << 24);
    unsigned is64   = frame[25];

    printf("[<] Identity frame (%d bytes):\n", frame_len);
    printf("    status  = %u\n", status);
    printf("    api     = %u, breed = %u (0=PIA, 1=this agent)\n", api, breed);
    printf("    uuid    = ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", frame[26 + i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) putchar('-');
    }
    printf("  (machine, .NET Guid order)\n");
    printf("    host    = \"%s\"\n", (const char *)(frame + 42));
    printf("    user    = \"%s\"\n", (const char *)(frame + 298));
    printf("    arch    = \"%s\"\n", (const char *)(frame + 554));
    printf("    platform= \"%s\"\n", (const char *)(frame + 586));
    printf("    os      = \"%s\"\n", (const char *)(frame + 618));
    printf("    build   = %u, commit = \"%s\", 64-bit = %u\n",
           build, (const char *)(frame + 12), is64);
    printf("    mask    = ");
    for (int i = 0; i < 8; i++)
        printf("%02x ", frame[746 + i]);
    printf("(categories: %s%s%s)\n",
           (frame[746] & 1) ? "FileSystem " : "",
           (frame[746] & 2) ? "Shell " : "",
           (frame[746] & 4) ? "Display" : "");
    if ((frame[746] & 7) == 0)
        printf("            (information-only agent)\n");

    DWORD dump_len = (frame_len < HEXDUMP_LIMIT) ? (DWORD)frame_len
                                                 : HEXDUMP_LIMIT;
    hex_dump(frame, dump_len);
    if ((DWORD)frame_len > dump_len)
        printf("    ... (%d more bytes not dumped)\n", frame_len - (int)dump_len);

    fflush(stdout);
}
