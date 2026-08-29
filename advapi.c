#include "advapi.h"
#include "system.h"
#include "apihash.h"

BOOL ADVAPI_Ctor(PADVAPI advapi)
{
    if (advapi == NULL)
        return FALSE;

    advapi->RegOpenKeyExA = (LSTATUS (WINAPI *)(HKEY, const PCHAR, DWORD, REGSAM, HKEY*))
        ResolveFromModuleByHash(HASH_MOD_ADVAPI32, HASH_REGOPENKEYEXA);
    advapi->RegQueryValueExA = (LSTATUS (WINAPI *)(HKEY, const PCHAR, DWORD*, DWORD*, unsigned char*, DWORD*))
        ResolveFromModuleByHash(HASH_MOD_ADVAPI32, HASH_REGQUERYVALUEEXA);
    advapi->RegCloseKey = (LSTATUS (WINAPI *)(HKEY))
        ResolveFromModuleByHash(HASH_MOD_ADVAPI32, HASH_REGCLOSEKEY);
    advapi->GetUserNameA = (BOOL (WINAPI *)(PCHAR, DWORD *))
        ResolveFromModuleByHash(HASH_MOD_ADVAPI32, HASH_GETUSERNAMEA);

    return (advapi->RegOpenKeyExA != NULL && advapi->RegQueryValueExA != NULL && advapi->RegCloseKey != NULL && advapi->GetUserNameA != NULL);
}
