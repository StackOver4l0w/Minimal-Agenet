/* shell.h - the cmd.exe pool (v4 contract: this agent owns shell identity).
 *
 * One live shell = one hidden cmd.exe behind two pipes. Stderr is merged
 * into the same pipe as stdout, so a single read drains everything and an
 * unread stderr can never grow until cmd.exe deadlocks. The pool itself
 * (an array of slots) is private to shell.c; ids sent to the panel are
 * slot indexes.
 */

#pragma once

/* No <windows.h>: every type this header needs (HANDLE, DWORD, the
 * STARTUPINFOW / PROCESS_INFORMATION / SECURITY_ATTRIBUTES shapes) lives
 * in the project's own minimal dictionary - types.h / wintypes.h. */
#include "types.h"
#include "wintypes.h"

#include "protocol.h"      /* SHELL_POOL_SIZE */

/* One pool slot. */
typedef struct {
    int     in_use;         /* slot occupied                        */
    HANDLE  stdin_w;        /* parent end: we WRITE commands here   */
    HANDLE  stdout_r;       /* parent end: we READ output here      */
    HANDLE  process;        /* cmd.exe process handle               */
} shell_slot;

/* Spawn cmd.exe behind two pipes and record it in *slot.
 * Returns 0 on success, non-zero on failure (handles already cleaned up). */
int shell_spawn(shell_slot *slot);

/* Find a free pool slot, spawn cmd.exe into it, and return its index
 * (the protocol shell id). Returns -1 when the pool is full or the spawn
 * failed (the slot stays free either way). */
int shell_open(void);

/* Release everything a slot owns. Safe on a partially-filled slot. */
void shell_teardown(shell_slot *slot);

/* Map a protocol shell id to its slot, or NULL when never opened/closed. */
shell_slot *shell_lookup(unsigned long long id);

/* Feed bytes to the shell's stdin.
 * Returns 0 on success; on failure the shell is dead and torn down. */
int shell_write(shell_slot *slot, const void *data, DWORD len);

/* Result codes for shell_read. */
#define SHELL_READ_OK       0   /* *out_len bytes are in out           */
#define SHELL_READ_IDLE     1   /* nothing buffered right now          */
#define SHELL_READ_DEAD     2   /* the shell process is gone           */

/* Drain up to cap bytes of buffered output, never blocking.
 * PeekNamedPipe first (how many bytes are waiting?), ReadFile second - a
 * direct ReadFile would block until output arrives, but this agent must be
 * free to answer the panel's next poll immediately. The panel's adaptive
 * polling (250..3000 ms) is the engine that drives the reads. */
int shell_read(shell_slot *slot, unsigned char *out, DWORD cap,
               DWORD *out_len);

/* Tear down every live shell (agent shutdown - no orphaned cmd.exe). */
void shell_teardown_all(void);
