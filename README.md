# minimal_agent — a dependency-free Windows agent

A small C program that connects to a relay over WebSocket, tells the
operator panel who it is (a 754-byte identity frame), and serves panel
commands inside a local `cmd.exe` (the Shell capability).

What makes it interesting is what it does NOT have:

- **no CRT** — no printf, no malloc, no startup code from the C runtime;
- **no import table** — not a single DLL is listed in the PE imports;
  every OS call, WinHTTP included, is found at runtime by walking the
  process's own module list (the PEB) and matching name hashes;
- **no strings in the binary** — API names live as precomputed hash
  constants inside instructions; the few strings the OS genuinely needs
  (the DLL name handed to the loader, the user agent, …) are built on
  the stack, XOR-decoded as they are written;
- **no `.bss`** — nothing static; everything lives on stack frames
  chosen so their lifetime matches what the data needs;
- **logging is a build option, not a feature** — a release build is
  silent; a dev build (`-DLOGGING_ENABLED`) talks.

The goal shape is a single-`.text` blob (raw shellcode); the section
scoreboard below shows where that journey stands.

---

## What you need

- Windows 8+ (WinHTTP's WebSocket API)
- **MinGW-w64 gcc** from MSYS2 (`ucrt64` environment). Nothing else -
  no SDK headers, no libraries: the project carries its own minimal
  type dictionary (`types.h`, `wintypes.h`) and resolves everything
  else at runtime.

---

## Build

Two steps: compile each `.c` into an object, then link the objects
into the exe. Compile and link flags must match (see the pair note
below).

### PowerShell (from the repo root)

```powershell
# Compile. Objects land in obj\ so the repo root stays clean.
#   -O2                        optimize (the flags below assume -O2)
#   -fno-asynchronous-...      no SEH unwind tables (.pdata/.xdata die)
#   -fno-shrink-wrap           required companion (see note below)
#   -fno-ident                 drop the compiler's signature strings
#   -DLOGGING_ENABLED          OPTIONAL: add this to get terminal logs
gcc -O2 -fno-asynchronous-unwind-tables -fno-shrink-wrap -fno-ident -c entry.c main.c transport.c shell.c report.c system_facts.c winhttp_api.c ntdll.c kernel32.c advapi.c string.c memory.c peb.c system.c djb2.c logger.c

New-Item -ItemType Directory -Force obj | Out-Null   # create obj\ (silent if exists)
Move-Item *.o obj                                     # move the fresh objects in

# Link. 
#   -s          strip symbols from the shipped exe
#   -nostdlib   no CRT - our entry.c is the startup
#   -e entry    THE entry point is our entry() (omitting this = instant crash)
gcc -O2 -s -fno-asynchronous-unwind-tables -fno-shrink-wrap -fno-ident -nostdlib -e entry -Wl,-T,link.text-first.ld -o minimal_agent.exe (Get-ChildItem obj\*.o | ForEach-Object FullName)
```

### cmd (classic Command Prompt)

```bat
:: same compile, one line - cmd has no line-continuation
gcc -O2 -fno-asynchronous-unwind-tables -fno-shrink-wrap -fno-ident -c entry.c main.c transport.c shell.c report.c system_facts.c winhttp_api.c ntdll.c kernel32.c advapi.c string.c memory.c peb.c system.c djb2.c logger.c

if not exist obj mkdir obj     & :: create obj\
move *.o obj                   & :: park the objects

gcc -O2 -s -fno-asynchronous-unwind-tables -fno-shrink-wrap -fno-ident -nostdlib -e entry -Wl,-T,link.text-first.ld -o minimal_agent.exe obj\*.o
```

### bash (MSYS2 shell)

```sh
gcc -O2 -fno-asynchronous-unwind-tables -fno-shrink-wrap -fno-ident -Iinclude -c entry.c src/main.c src/transport.c src/shell.c src/report.c src/system_facts.c src/winhttp_api.c src/ntdll.c src/kernel32.c src/advapi.c src/string.c src/memory.c src/peb.c src/system.c src/djb2.c src/logger.c src/environment.c
mkdir -p obj && mv *.o obj/                      # objects out of the root
gcc -O2 -s -fno-asynchronous-unwind-tables -fno-shrink-wrap -fno-ident -nostdlib -e entry -Wl,-T,link.text-first.ld -o minimal_agent.exe obj/*.o
```

### Why the two `-fno-*` flags travel as a pair

`-fno-asynchronous-unwind-tables` removes the SEH unwind tables the
agent never uses. But the moment they are gone, gcc is allowed to
*shrink-wrap* prologues - the `push`/`sub rsp` moves into the middle
of a function - and it rebuilds the prologue of every function; the
code GREW by 1 KB when measured. `-fno-shrink-wrap` forbids that.
The pair costs +64 bytes total instead.

---

## Check your build (the two gates)

**Gate 1 - the import table must be EMPTY.** A single `DLL Name:`
line means something pulled a library back in.

```powershell
# PowerShell / cmd (findstr needs /C: for a literal phrase with a space,
# otherwise it matches "DLL" OR "Name" and prints the table header):
objdump -p minimal_agent.exe | findstr /C:"DLL Name:"
# prints NOTHING = pass
```

```sh
# bash:
objdump -p minimal_agent.exe | grep "DLL Name"     # no output = pass
```

**Gate 2 - the entry point must be OURS.** The shipped exe is stripped
(`-s`), so `nm` sees no symbols in it - link an unstripped CHECK copy
of the same objects and compare two addresses:

```powershell
# PowerShell: build a check copy, then ask nm where the entry symbol is
gcc -nostdlib -e entry -Wl,-T,link.text-first.ld -o ma_check.exe (Get-ChildItem obj\*.o | ForEach-Object FullName)
nm ma_check.exe | findstr /C:" T entry"         # prints e.g. 0000000140004aa0 T entry
objdump -f ma_check.exe | findstr /C:"start address"   # start address 0x...4aa0
del ma_check.exe                                # same number twice = pass
```

```sh
# bash:
gcc -nostdlib -e entry -Wl,-T,link.text-first.ld -o /tmp/ma_check.exe obj/*.o
nm /tmp/ma_check.exe | grep " T entry"          # -> ...4aa0 T entry
objdump -f /tmp/ma_check.exe | grep "start address"   # -> same address = pass
```

If gate 2 fails, you forgot `-e entry` or lost `entry.o` from the list:
the binary would build fine and crash instantly at startup.

### Check the sections (the scoreboard)

The whole point of this project's shape; see where it stands:

```sh
objdump -h minimal_agent.exe        # list every section with its size
```

The current scoreboard (release build):

| Section   | Size | Meaning |
|---|---|---|
| `.text`   | ~15 KB | the code - everything the agent is, and ALL there is |

That is the whole table: one section. The linker script
(`link.text-first.ld`) merges every code piece into a single `.text`
and puts `entry()` at byte zero - the address a raw-blob loader jumps
to. A quick eyeball in PowerShell:

```powershell
objdump -h minimal_agent.exe | findstr /C:".text" /C:".rdata" /C:".bss" /C:".pdata"
```

---

## Run

```
minimal_agent.exe <relay-url>        # e.g. https://relay.example.com/agent
minimal_agent.exe <relay-url> -v     # verbose: also dump every command raw (dev builds)
```

**A release build prints nothing.** Not even errors. It connects,
identifies, and serves; the console just sits there while it works -
that silence is the point of the release flavor.

To actually SEE what it does, build the DEV flavor: the same two
steps as the release build with `-DLOGGING_ENABLED` added to BOTH
the compile and the link line (miss one and you get a silent hybrid),
and a distinct output name so the two exes do not overwrite each
other:

```powershell
# 1) compile - same flags, plus -DLOGGING_ENABLED
gcc -O2 -fno-asynchronous-unwind-tables -fno-shrink-wrap -fno-ident -DLOGGING_ENABLED -c entry.c main.c transport.c shell.c report.c system_facts.c winhttp_api.c ntdll.c kernel32.c advapi.c string.c memory.c peb.c system.c djb2.c logger.c

# 2) park the objects in obj\ (separate dir per flavor keeps them apart)
New-Item -ItemType Directory -Force obj | Out-Null
Move-Item *.o obj

# 3) link - the flag AGAIN here, and a distinct name
gcc -O2 -s -fno-asynchronous-unwind-tables -fno-shrink-wrap -fno-ident -DLOGGING_ENABLED -nostdlib -e entry -Wl,-T,link.text-first.ld -o minimal_agent_dev.exe (Get-ChildItem obj\*.o | ForEach-Object FullName)

# 4) run the talkative one
.\minimal_agent_dev.exe https://relay.example.com/agent
```

It prints:

```
[INF] Connecting to https://relay.example.com/agent ...
[INF] Connected (HTTP 101 Switching Protocols)

[INF] [2] Agent mode: replying to commands (capability mask = Shell)...

[INF] Identity sent to the panel (754 bytes)

```

With `-v` (dev build) every incoming command is additionally dumped:
opcode name, decoded payload, a hex dump, and the outgoing identity
frame field by field.

### What it does on the wire

- connects and upgrades HTTP to WebSocket (an ordinary HTTPS GET with
  `WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET`; the `101 Switching
  Protocols` response completes the handshake);
- `Hello` -> the 754-byte identity frame: machine UUID (registry
  MachineGuid in .NET Guid byte order), hostname, user, OS version,
  build metadata, and a capability mask with Shell set;
- `OpenShell` -> spawns a hidden `cmd.exe` (code page UTF-8) behind
  two pipes; the slot index IS the shell id (256 slots);
- `WriteShell` / `ReadShell` -> feed input / drain output; reads never
  block, the panel's polling drives the flow;
- `CloseShell` -> kill the shell, free the slot;
- `Exit` -> terminate the agent (the only command with no reply);
- anything else -> `status = 1`.

A lost connection is normal, not an error: the agent redials after a
backoff (1..32 s, reset after a healthy session). Live shells survive
a redial - their pool belongs to the process, not the connection.

Because the agent advertises Shell without FileSystem, the panel's
file manager falls back to PowerShell-over-shell - a basic file
browser works with zero file opcodes implemented.

---

## How the code is laid out

| File | What it owns |
|---|---|
| `entry.c` | the real process entry: zero `.bss` (a no-op while empty), build argv from the PEB, call agent_main, exit via ExitProcess |
| `main.c` | `agent_main`: owns process-lifetime state on its frame (shell pool, backoff), bundles it into `agent_ctx`, runs dial/serve/redial |
| `protocol.h` | opcodes, statuses, the identity frame layout, capability mask |
| `wire.h` | tiny little-endian writers (header-only) |
| `transport.h/.c` | the WebSocket pipe: one reply out (`ws_send`), one assembled message in (`ws_receive`) |
| `shell.h/.c` | the cmd.exe pool: spawn / write / drain / teardown, 256 slots |
| `report.h/.c` | terminal diagnostics - compiles to NOTHING unless `LOGGING_ENABLED` |
| `system_facts.h/.c` | machine UUID, hostname, username, OS version (the identity payload) |
| `winhttp_api.h/.c` | the WinHTTP table + the LdrLoadDll bootstrap that maps winhttp.dll |
| `kernel32/ntdll/advapi.h/.c` | one function table per DLL, hash-resolved |
| `peb.h/.c` | TEB/PEB access, the loader module-list walk |
| `system.h/.c` | export-table resolve - by name (tooling) and by hash (the agent) |
| `apihash.h` | the precomputed djb2 constants for every name used |
| `stackstrings.h` | every runtime string, built on the stack, XOR-decoded in the write |
| `djb2.h/.c` | the hash both resolve paths share |
| `string.c` / `memory.c` / `logger.c` | the hand-rolled CRT replacements |

Reading order for a newcomer: `entry.c` (how a process starts without
a runtime) -> `peb.c` + `system.c` (how functions are found without
imports) -> `transport.c` (the wire) -> `main.c` (the loop).

---

## CI / Releases (GitHub Actions)

Cross-builds the three Windows architectures with
[llvm-mingw](https://github.com/mstorsjo/llvm-mingw) (i686 / x86_64 /
aarch64) and bakes the identity metadata (`-DID_BUILD_NUMBER`,
`-DAGENT_COMMIT_HASH`):

- **build.yml** — on push/PR: compile check; on pushes to main also
  republishes the rolling `preview` pre-release
  (`windows-i386.exe`, `windows-x86_64.exe`, `windows-aarch64.exe`);
- **release.yml** — on a `v*` tag: the same binaries as a stable
  GitHub Release.

---

## Honest limitations

- Shell only: no native file or screen opcodes (the panel covers files
  via its PowerShell-over-shell fallback); shells die with the process.
- WinHTTP is loaded at runtime but visible in the process's module
  list for its whole life (removing it is future work).
- The connection pattern (periodic dial to one host, a non-browser
  user agent) is trivially visible to network monitoring - deliberate
  scope: this project studies form, not evasion.
- The `.rdata`/`.idata` tails still exist as unreferenced sections;
  the remaining step toward the single-`.text` blob is linker work.
