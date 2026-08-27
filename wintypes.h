#pragma once
#include "types.h"

typedef enum _WINHTTP_WEB_SOCKET_BUFFER_TYPE
{
    WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE       = 0,
    WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE      = 1,
    WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE         = 2,
    WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE        = 3,
    WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE                = 4
} WINHTTP_WEB_SOCKET_BUFFER_TYPE;


/* The UINT32 pad is load-bearing on x64: without it Buffer lands at
 * offset 6 while the real loader layout puts it at 8 - every struct
 * embedding UNICODE_STRING (RTL_USER_PROCESS_PARAMETERS, LDR entries)
 * then skews and we read garbage pointers. */
typedef struct _UNICODE_STRING
{
	UINT16 Length;        ///< Length of the string in bytes (not including any null terminator)
	UINT16 MaximumLength; ///< Total size of the Buffer in bytes
	UINT32 Padding;       ///< Alignment so Buffer sits at offset 8 (x64)
	PWCHAR Buffer;        ///< Pointer to the wide character string data
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _SECURITY_ATTRIBUTES {
  DWORD nLength;
  PVOID lpSecurityDescriptor;
  BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _STARTUPINFOW {
  DWORD  cb;
  PWCHAR lpReserved;
  PWCHAR lpDesktop;
  PWCHAR lpTitle;
  DWORD  dwX;
  DWORD  dwY;
  DWORD  dwXSize;
  DWORD  dwYSize;
  DWORD  dwXCountChars;
  DWORD  dwYCountChars;
  DWORD  dwFillAttribute;
  DWORD  dwFlags;
  UINT16   wShowWindow;
  UINT16   cbReserved2;
  unsigned char *lpReserved2;
  HANDLE hStdInput;
  HANDLE hStdOutput;
  HANDLE hStdError;
} STARTUPINFOW, *LPSTARTUPINFOW;

typedef struct _PROCESS_INFORMATION {
  HANDLE hProcess;
  HANDLE hThread;
  DWORD  dwProcessId;
  DWORD  dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;

typedef struct _OSVERSIONINFOW {
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  WCHAR szCSDVersion[128];
} OSVERSIONINFOW, *POSVERSIONINFOW, *LPOSVERSIONINFOW, RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;

