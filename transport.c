/* transport.c - the WebSocket pipe (see transport.h).
 */

#include "transport.h"

/* Send one binary reply. Returns the WinHTTP error code (0 = success). */
DWORD ws_send(HINTERNET socket, const void *data, DWORD length)
{
    return WinHttpWebSocketSend(socket,
                                WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
                                (PVOID)data, length);
}

/* Receive and assemble one complete WebSocket message (see transport.h). */
DWORD ws_receive(HINTERNET socket, incoming_message *msg, BOOL *closed)
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
