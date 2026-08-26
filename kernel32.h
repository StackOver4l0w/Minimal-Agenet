#include "ntdll.h"
#include "djb2.h"
#include "peb.h"
#include "system.h"
#include "wintypes.h"

#define CREATE_NO_WINDOW 0x08000000
#define HANDLE_FLAG_INHERIT    0x00000001
#define STARTF_USESTDHANDLES   0x00000100

typedef struct KERNEL32
{
    PVOID (WINAPI *GetProcAddress)(PVOID ModuleHandle, const CHAR *ProcName);
    PVOID (WINAPI *LoadLibraryA)(const CHAR *LibFileName);
    BOOL (WINAPI *GetComputerNameA)(PCHAR Buffer, DWORD *Size);
    BOOL (WINAPI *SetHandleInformation)(HANDLE hObject, DWORD dwMask, DWORD dwFlags);
    BOOL (WINAPI *CreateProcessW)(const PWCHAR lpApplicationName, const PWCHAR lpCommandLine,LPSECURITY_ATTRIBUTES lpProcessAttributes,LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles,
    DWORD dwCreationFlags, PVOID lpEnvironment, const PWCHAR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
    BOOL (WINAPI *CloseHandle)(HANDLE hObject);
    BOOL (WINAPI *TerminateProcess)(HANDLE hProcess, UINT32 uExitCode);
    BOOL (WINAPI *WriteFile)(HANDLE hFile, const void *lpBuffer, DWORD nNumberOfBytesToWrite, DWORD *lpNumberOfBytesWritten, PVOID lpOverlapped);
    BOOL (WINAPI *ReadFile)(HANDLE hFile, void *lpBuffer, DWORD nNumberOfBytesToRead, DWORD *lpNumberOfBytesRead, PVOID lpOverlapped);
    BOOL (WINAPI *PeekNamedPipe)(HANDLE hNamedPipe, void *lpBuffer, DWORD nBufferSize, DWORD *lpBytesRead, DWORD *lpTotalBytesAvail, DWORD *lpBytesLeftThisMessage);
    BOOL (WINAPI *CreatePipe)(HANDLE *hReadPipe, HANDLE *hWritePipe, LPSECURITY_ATTRIBUTES lpPipeAttributes, DWORD nSize);
    HANDLE (WINAPI *GetStdHandle)(DWORD nStdHandle);
    DWORD (WINAPI *GetLastError)(void);
    void (WINAPI *Sleep)(DWORD dwMilliseconds);
} KERNEL32;

BOOL KERNEL32_Ctor(KERNEL32 *kernel);