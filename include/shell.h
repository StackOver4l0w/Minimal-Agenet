#pragma once

#include "types.h"
#include "wintypes.h"

#include "protocol.h"

typedef struct {
    int     in_use;
    HANDLE  stdin_w;
    HANDLE  stdout_r;
    HANDLE  process;
} shell_slot;

int shell_spawn(shell_slot *slot);

#define SHELL_POOL shell_slot pool[SHELL_POOL_SIZE]

int shell_open(shell_slot pool[]);

void shell_teardown(shell_slot *slot);

shell_slot *shell_lookup(shell_slot pool[], unsigned long long id);

int shell_write(shell_slot *slot, const void *data, DWORD len);

#define SHELL_READ_OK       0
#define SHELL_READ_IDLE     1
#define SHELL_READ_DEAD     2

int shell_read(shell_slot *slot, unsigned char *out, DWORD cap,
               DWORD *out_len);

void shell_teardown_all(shell_slot pool[]);
