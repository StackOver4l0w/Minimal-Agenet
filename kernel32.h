#include "ntdll.h"
#include "djb2.h"
#include "peb.h"
#include "system.h"

typedef struct KERNEL
{
    PVOID (WINAPI *GetProcAddress)(PVOID ModuleHandle, const CHAR *ProcName);
    PVOID (WINAPI *LoadLibraryA)(const CHAR *LibFileName);
    BOOL (WINAPI *GetComputerNameA)(PCHAR Buffer, DWORD *Size);
} KERNEL;

BOOL KERNEL_Ctor(KERNEL *kernel);