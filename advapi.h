#include "types.h"


#define HKEY_CLASSES_ROOT                   ((HKEY)(ULONG_PTR)0x80000000u)
#define HKEY_CURRENT_USER                   ((HKEY)(ULONG_PTR)0x80000001u)
#define HKEY_LOCAL_MACHINE                  ((HKEY)(ULONG_PTR)0x80000002u)
#define HKEY_USERS                          ((HKEY)(ULONG_PTR)0x80000003u)
#define HKEY_PERFORMANCE_DATA               ((HKEY)(ULONG_PTR)0x80000004u)
#define HKEY_PERFORMANCE_TEXT               ((HKEY)(ULONG_PTR)0x80000050u)
#define HKEY_PERFORMANCE_NLSTEXT            ((HKEY)(ULONG_PTR)0x80000060u)
#define KEY_QUERY_VALUE 0x0001

#define REG_SZ 1

typedef struct ADVAPI{
    LSTATUS (WINAPI *RegOpenKeyExA)(HKEY hKey, const PCHAR lpSubKey, DWORD ulOptions, REGSAM samDesired, HKEY* phkResult);
    LSTATUS (WINAPI *RegQueryValueExA)(HKEY hKey, const PCHAR lpValueName, DWORD* lpReserved, DWORD* lpType, unsigned char* lpData, DWORD* lpcbData);
    LSTATUS (WINAPI *RegCloseKey)(HKEY hKey);
} ADVAPI, *PADVAPI;

BOOL ADVAPI_Ctor(PADVAPI advapi);