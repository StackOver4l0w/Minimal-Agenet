/* report.h - everything printed for the human watching the terminal.
 *
 * Diagnostics (error codes), hex dumps, the incoming-command decoder and
 * the outgoing identity-frame printer. If it writes to stdout/stderr and
 * is not part of the dispatch flow itself, it lives here.
 */

#pragma once

#include "wintypes.h"

#include "protocol.h"
#include "transport.h"     /* incoming_message */

/* WINHTTP_WEB_SOCKET_BUFFER_TYPE is defined once, in wintypes.h (shared
 * by transport.h and this header); a copy of the enum here broke clang
 * with a redefinition error. */

/* WinHTTP's WebSocket buffer type, as a string. WinHTTP does not expose
 * RFC 6455 frame opcodes; this enum is the only "type" the application
 * ever sees (ping/pong is handled internally). */
const char *ws_buffer_type_name(WINHTTP_WEB_SOCKET_BUFFER_TYPE type);

/* The relay-protocol name of an incoming command opcode. */
const char *command_name(unsigned char opcode);

/* Classic 16-bytes-per-line hex dump with an ASCII column. */
void hex_dump(const unsigned char *data, DWORD length);

/* Decode the command-specific part (everything after the 1-byte opcode) so
 * the terminal says WHAT the panel asked for, not just raw bytes. */
void describe_command_payload(unsigned char opcode,
                              const unsigned char *p, DWORD len);

/* Print one received command: header, decoded opcode and payload, hex
 * dump, printable text. Explicit length - the payload is NOT a C string. */
void print_command(int index, const incoming_message *msg);

/* Print the outgoing identity frame field by field (the mirror of
 * print_command: everything WE send is shown too, not just what we get). */
void print_identity_frame(const unsigned char frame[IDENTITY_FRAME_SIZE],
                          int frame_len);
