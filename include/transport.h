#pragma once

#include "types.h"
#include "wintypes.h"
#include "protocol.h"
#include "winhttp_api.h"

#define ERROR_MOD_NOT_FOUND 126

DWORD ws_send(const WINHTTP_API *api, HINTERNET socket, const void *data, DWORD length);

typedef struct {
    unsigned char data[MAX_MESSAGE_SIZE];
    DWORD length;
    BOOL truncated;
    WINHTTP_WEB_SOCKET_BUFFER_TYPE type;
} incoming_message;

DWORD ws_receive(const WINHTTP_API *api, HINTERNET socket, incoming_message *msg, BOOL *closed);
