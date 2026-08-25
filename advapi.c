#include "advapi.h"
#include "system.h"

BOOL ADVAPI_Ctor(PADVAPI advapi)
{
    if (advapi == NULL)
        return FALSE;

    advapi->RegOpenKeyExA = (LSTATUS (WINAPI *)(HKEY, const PCHAR, DWORD, REGSAM, HKEY*))
        ResolveFromModuleByName(L"advapi32.dll", "RegOpenKeyExA");

    return (advapi->RegOpenKeyExA != NULL);
}