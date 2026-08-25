#include "ntdll.h"
#include "djb2.h"
#include "peb.h"
#include "system.h"
#include "wintypes.h"

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
} KERNEL32;

BOOL KERNEL32_Ctor(KERNEL32 *kernel);