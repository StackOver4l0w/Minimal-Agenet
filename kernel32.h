#include "ntdll.h"
#include "djb2.h"
#include "peb.h"
#include "system.h"

typedef struct KERNEL32
{
    PVOID (WINAPI *GetProcAddress)(PVOID ModuleHandle, const CHAR *ProcName);
    PVOID (WINAPI *LoadLibraryA)(const CHAR *LibFileName);
    BOOL (WINAPI *GetComputerNameA)(PCHAR Buffer, DWORD *Size);
    BOOL (WINAPI *SetHandleInformation)(HANDLE hObject, DWORD dwMask, DWORD dwFlags);
} KERNEL32;

BOOL KERNEL32_Ctor(KERNEL32 *kernel);