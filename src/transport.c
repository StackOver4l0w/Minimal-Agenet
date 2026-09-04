#include "transport.h"
#include "memory.h"


DWORD ws_send(const WINHTTP_API *api, HINTERNET socket, const void *data, DWORD length)
{
    if (api == NULL || api->WinHttpWebSocketSend == NULL || (data == NULL && length != 0))
        return ERROR_MOD_NOT_FOUND;

    return api->WinHttpWebSocketSend(socket,
                                     WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
                                     (void*)data, length);
}

DWORD ws_receive(const WINHTTP_API *api, HINTERNET socket, incoming_message *msg, BOOL *closed)
{
    if (api == NULL || msg == NULL || closed == NULL || api->WinHttpWebSocketReceive == NULL)
        return ERROR_MOD_NOT_FOUND;

    MemoryZero(msg, sizeof(*msg));
    *closed = FALSE;

    for (;;) {
        unsigned char fragment[RECV_FRAGMENT_SIZE];
        DWORD got = 0;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE type;

        DWORD err = api->WinHttpWebSocketReceive(socket, fragment, sizeof(fragment), &got, &type);
        if (err != NO_ERROR)
            return err;

        if (type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE) {
            *closed = TRUE;
            return NO_ERROR;
        }

        if (got > 0) {
            if (got <= MAX_MESSAGE_SIZE - msg->length) {
                MemoryCopy(msg->data + msg->length, fragment, got);
                msg->length += got;
            } else {
                msg->truncated = TRUE;
            }
        }

        if (type == WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE || type == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE) {
            msg->type = type;
            return NO_ERROR;
        }
    }
}
