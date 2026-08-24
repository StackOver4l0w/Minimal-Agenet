#include "types.h"
typedef enum _WINHTTP_WEB_SOCKET_BUFFER_TYPE
{
    WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE       = 0,
    WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE      = 1,
    WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE         = 2,
    WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE        = 3,
    WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE                = 4
} WINHTTP_WEB_SOCKET_BUFFER_TYPE;


typedef struct _UNICODE_STRING
{
	UINT16 Length;        ///< Length of the string in bytes (not including any null terminator)
	UINT16 MaximumLength; ///< Total size of the Buffer in bytes
	PWCHAR Buffer;        ///< Pointer to the wide character string data
} UNICODE_STRING, *PUNICODE_STRING;