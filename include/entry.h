/* entry.h - CRT-free process entry (the -nostdlib stage).
 *
 * With -nostdlib the mingw startup is gone, so this module provides
 * exactly what the agent used it for:
 *
 *   entry()          - the real PE entry point: zeroes .bss, builds
 *                      argv from PEB->ProcessParameters->CommandLine,
 *                      calls agent_main, exits via ExitProcess
 *   __chkstk / ___chkstk_ms() - the stack-probe symbols x64 Windows emits
 *                      for frames > 4 KB (our 64 KB WebSocket buffers qualify)
 *
 * main.c renames its main() to agent_main() - the CRT-free contract.
 */

#ifndef ENTRY_H
#define ENTRY_H

#include "types.h"

/* Implemented in entry.c; main.c provides the body. */
INT32 agent_main(const WCHAR *url);

#endif /* ENTRY_H */