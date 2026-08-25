#include "advapi.h"
#include "system.h"

BOOL ADVAPI_Ctor(PADVAPI advapi)
{
    if (advapi == NULL)
        return FALSE;

    advapi->RegOpenKeyExA = (LSTATUS (WINAPI *)(HKEY, const PCHAR, DWORD, REGSAM, HKEY*))
        ResolveFromModuleByName(L"advapi32.dll", "RegOpenKeyExA");
    advapi->RegQueryValueExA = (LSTATUS (WINAPI *)(HKEY, const PCHAR, DWORD*, DWORD*, unsigned char*, DWORD*))
        ResolveFromModuleByName(L"advapi32.dll", "RegQueryValueExA");

    return (advapi->RegOpenKeyExA != NULL && advapi->RegQueryValueExA != NULL);
}