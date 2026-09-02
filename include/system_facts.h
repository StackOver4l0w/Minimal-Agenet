#pragma once

#include "types.h"
#include "protocol.h"

void get_machine_uuid(unsigned char out[16]);

typedef struct {
    char hostname[ID_HOSTNAME_SIZE];
    char username[ID_USERNAME_SIZE];
    char os_version[ID_OS_VERSION_SIZE];
} system_facts;

void collect_system_facts(system_facts *facts);
