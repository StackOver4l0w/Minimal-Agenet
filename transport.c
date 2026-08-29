/* transport.c - the WebSocket pipe (see transport.h).
 */

#include "transport.h"
#include "types.h"
#include "memory.h"
#include "winhttp_api.h"

/* С1 step 2: the lazy static table is GONE. The WinHTTP table is built
 * ONCE per session by run_session() (its frame owns it) and passed down
 * as a parameter - the same owner-on-top model as the shell pool. This
 * keeps the colleague's "resolve once, not per call" property (the PEB
 * walk still happens exactly once per session) while removing .bss. */

/* Send one binary reply. Returns the WinHTTP error code (0 = success). */
DWORD ws_send(const WINHTTP_API *api, HINTERNET socket, const void *data, DWORD length)
{
    if (api == NULL || api->WinHttpWebSocketSend == NULL)
        return ERROR_MOD_NOT_FOUND;   /* nonzero = failure per the family's
                                         direct-error-code contract */

    return api->WinHttpWebSocketSend(socket,
                                     WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
                                     (PVOID)data, length);
}

/* Receive and assemble one complete WebSocket message (see transport.h). */
DWORD ws_receive(const WINHTTP_API *api, HINTERNET socket, incoming_message *msg, BOOL *closed)
{
    if (api == NULL || api->WinHttpWebSocketReceive == NULL)
        return ERROR_MOD_NOT_FOUND;

    MemoryZero(msg, sizeof(*msg));
    *closed = FALSE;

    for (;;) {
        unsigned char fragment[RECV_FRAGMENT_SIZE];   /* С1 step 2: frame-local */
        DWORD got = 0;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE type;

        DWORD err = api->WinHttpWebSocketReceive(socket, fragment,
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
