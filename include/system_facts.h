/* system_facts.h - what this agent reports about its machine.
 *
 * Two producers feed the identity frame: the machine UUID (registry
 * MachineGuid, reordered to the .NET Guid layout the panel parses) and the
 * printable facts (hostname, user, OS version).
 */

#pragma once

#include "types.h"
#include "protocol.h"      /* identity-frame field widths */

/* Fill out[16] with the machine UUID from HKLM\...\Cryptography\MachineGuid,
 * converted to the .NET Guid byte order the panel expects. Falls back to
 * all zeros when the value is missing or malformed. */
void get_machine_uuid(unsigned char out[16]);

/* Everything the identity frame needs to know about the machine. */
typedef struct {
    char hostname[ID_HOSTNAME_SIZE];
    char username[ID_USERNAME_SIZE];
    char os_version[ID_OS_VERSION_SIZE];
} system_facts;

void collect_system_facts(system_facts *facts);
