#include "types.h"
#include "wintypes.h"

#ifndef WINAPI
#define WINAPI
#endif

typedef struct NTDLL
{
    NTSTATUS (WINAPI *LdrLoadDll)(
        WCHAR *PathToFile,
        UINT32 Flags,
        PUNICODE_STRING ModuleFileName,
        PVOID *ModuleHandle
    );

    NTSTATUS (WINAPI *RtlGetVersion)(PVOID VersionInformation);

} NTDLL;

BOOL NTDLL_Ctor(NTDLL* ntdll);

