#include "types.h"

#define HKEY_LOCAL_MACHINE   ((HKEY)(ULONG_PTR)0x80000002u)
#define KEY_QUERY_VALUE      0x0001
#define REG_SZ                1

typedef struct ADVAPI{
    LSTATUS (WINAPI *RegOpenKeyExA)(HKEY hKey, const PCHAR lpSubKey, DWORD ulOptions, REGSAM samDesired, HKEY* phkResult);
    LSTATUS (WINAPI *RegQueryValueExA)(HKEY hKey, const PCHAR lpValueName, DWORD* lpReserved, DWORD* lpType, unsigned char* lpData, DWORD* lpcbData);
    LSTATUS (WINAPI *RegCloseKey)(HKEY hKey);
    BOOL (WINAPI *GetUserNameA)(PCHAR lpBuffer, DWORD *nSize);
} ADVAPI, *PADVAPI;

BOOL ADVAPI_Ctor(PADVAPI advapi);
