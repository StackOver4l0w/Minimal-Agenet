#pragma once

#include "wintypes.h"
#include "protocol.h"
#include "transport.h"

const char *ws_buffer_type_name(WINHTTP_WEB_SOCKET_BUFFER_TYPE type);
const char *command_name(unsigned char opcode);
void hex_dump(const unsigned char *data, DWORD length);
void describe_command_payload(unsigned char opcode, const unsigned char *p, DWORD len);
void print_command(int index, const incoming_message *msg);

