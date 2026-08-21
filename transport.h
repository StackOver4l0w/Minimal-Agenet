/* transport.h - the WebSocket pipe: send one reply, receive one command.
 *
 * WinHTTP specifics live here: the asymmetric error contract (classic
 * WinHTTP calls return BOOL + GetLastError; the WinHttpWebSocket* family
 * returns the error code directly) and the fragment/message distinction
 * (one Receive call yields one fragment; a message may span several).
 */

#pragma once

#include <windows.h>
#include <winhttp.h>

#include "protocol.h"      /* MAX_MESSAGE_SIZE */

/* Send one binary reply. Returns the WinHTTP error code (0 = success). */
DWORD ws_send(HINTERNET socket, const void *data, DWORD length);

/* One assembled incoming message. */
typedef struct {
    unsigned char data[MAX_MESSAGE_SIZE];
    DWORD length;
    BOOL truncated;               /* message exceeded MAX_MESSAGE_SIZE */
    WINHTTP_WEB_SOCKET_BUFFER_TYPE type;
} incoming_message;

/* Receive and assemble one complete WebSocket message. One
 * WinHttpWebSocketReceive call yields one fragment; a message may span
 * several, and the last fragment carries the *_MESSAGE type. Returns the
 * WinHTTP error code (0 = success). If the server sent a close frame, the
 * return is 0 and *closed is set; the caller decides what to do. */
DWORD ws_receive(HINTERNET socket, incoming_message *msg, BOOL *closed);
