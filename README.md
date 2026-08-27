# minimal_agent — a dependency-free Windows agent

A small Windows agent in C that connects to a relay over WebSocket,
identifies itself to the operator panel (754-byte v5 Hello frame), and
runs panel commands inside a local `cmd.exe` (Shell category).

The defining property: **the binary has an empty import table**. No
CRT, no `winhttp.dll`, no `kernel32.dll` entries — nothing. Every OS
facility is resolved at runtime by reading the process's own loader
structures (the PEB) and walking PE export tables. The process starts
at a hand-written entry point, not the C runtime startup.

```
$ objdump -p minimal_agent.exe | grep "DLL Name"
$            <- nothing: zero imports
```

This is a study project: every layer a normal program gets "for free"
(the C runtime, `printf`, the linker's startup, the import table) is
built by hand here, and every hand-built piece was verified against
the real system function it replaces.

## What it does

A "minimal agent" in the relay protocol is anything that answers the
panel's Hello command with an identity frame. Everything else is
optional, advertised through a capability mask. This agent implements
the **Shell** category:

- `0x00 Hello` → replies with the 754-byte v5 identity frame
  (metadata-first: status, API version, breed id, commit hash, build
  number, 64-bit flag, then UUID + machine facts, mask last);
- `0x0A OpenShell` → spawns a hidden `cmd.exe` (UTF-8 code page) wired
  to two pipes, returns a shell id from a 256-slot pool;
- `0x04/0x05 Write/ReadShell` → feed operator input / drain buffered
  output back to the panel;
- `0x08 CloseShell` → terminates the shell, frees the slot;
- `0x09 Exit` → terminates the agent (the only command with no reply);
- anything else → `status = 1` ("not implemented").

A lost connection is a normal event, not an error: the agent redials
with a capped backoff (1..32 s, reset after a healthy session). Live
shells survive a redial. A side effect of advertising Shell without
FileSystem: the panel's file manager falls back to its
PowerShell-over-shell backend, so a basic file browser works too.

## How the dependency removal works

Three ideas replace the usual exe scaffolding:

1. **PEB walk** (`peb.c`, `system.c`) — the loader's own module list
   gives the base address of any loaded DLL; its export table gives
   function addresses. No import table needed.
2. **`LdrLoadDll` bootstrap** (`winhttp_api.c`) — with no static
   import, `winhttp.dll` is not in the module list at startup. The
   agent resolves `LdrLoadDll` from ntdll (always present — it *is*
   the loader) and maps winhttp itself, then resolves the table.
3. **Own runtime** (`entry.c`, `memory.c`, `string.c`, `logger.c`) —
   a hand-written process entry (zero `.bss` by walking the PE section
   table, argv from `PEB->ProcessParameters`, exit via `ExitProcess`),
   a `Format`-based logger over `WriteFile`, volatile byte loops
   instead of `memcpy`/`memset` (the compiler recognizes plain loops
   and substitutes CRT calls), and a stack probe transcribed verbatim
   from libgcc's `_chkstk_ms`.

Each vtable module (`kernel32.h`, `ntdll.h`, `advapi.h`,
`winhttp_api.h`) follows one pattern: a struct of function pointers
filled by a constructor, stack-local per call site — no globals.

## Build

Toolchain: MinGW-w64 gcc (tested with 16.1, MSYS2 `ucrt64`). Two
steps — compile, then link:

```sh
gcc -O2 -c main.c transport.c shell.c report.c system_facts.c \
    winhttp_api.c memory.c string.c kernel32.c advapi.c ntdll.c \
    peb.c system.c djb2.c logger.c entry.c freestanding.c

gcc -O2 -s -nostdlib -e entry -o minimal_agent.exe \
    entry.o main.o transport.o shell.o report.o system_facts.o \
    winhttp_api.o memory.o string.o kernel32.o advapi.o ntdll.o \
    peb.o system.o djb2.o logger.o freestanding.o
```

Two details that matter:
- `entry.c` must be in the object list — it provides `___chkstk_ms`
  and the entry point;
- `-e entry` names the real entry — without it the PE header points
  at the wrong symbol and the binary crashes at start.

Verify the result:

```sh
objdump -p minimal_agent.exe | grep "DLL Name"       # expect: nothing
objdump -f minimal_agent.exe | grep "start address"  # must equal: nm minimal_agent.exe | grep " T entry"
```

x86_64 only, deliberately: the entry layer (stack probe, PEB layout
offsets) is proven live on x86_64. i386 needs a 32-bit
`RTL_USER_PROCESS_PARAMETERS` layout and its own probe; aarch64 needs
an ARM64 probe — both are future work.

## Run

```
minimal_agent.exe <URL>          # one line per event
minimal_agent.exe <URL> -v       # verbose: dump every command's raw bytes
```

```
[INF] Connecting to https://relay.example.com/agent ...
[INF] Connected (HTTP 101 Switching Protocols)
[INF] [2] Agent mode: replying to commands (capability mask = Shell)...
[INF] Identity sent to the panel (754 bytes)
[INF] Shell 0 opened (cmd.exe spawned) - id sent
```

After the identity is sent the agent is quiet until the panel asks —
serving is silent by design. Terminate with Ctrl+C or the panel's
Exit command.

## Module map (one topic per file)

| File | Topic |
|---|---|
| `main.c` | `agent_main`: connect, dispatch commands, cleanup |
| `entry.h/.c` | CRT-free process entry: `.bss` zeroing via PE section walk, argv from PEB, `___chkstk_ms` stack probe, `ExitProcess` exit |
| `peb.h/.c` | PEB structures + the loader-list walk |
| `system.h/.c` | PE export resolver: `ResolveExportByName` / `ResolveFromModuleByName` |
| `djb2.h/.c` | djb2 hash over wide/ascii strings |
| `kernel32/ntdll/advapi/winhttp_api .h/.c` | runtime-resolved API vtables |
| `transport.h/.c` | the WebSocket pipe: `ws_send` / `ws_receive` |
| `shell.h/.c` | the cmd.exe pool: spawn / read / write / teardown |
| `report.h/.c` | human-facing output: hex dumps, command decoders |
| `logger.h/.c` | `LOG_INFO`/`LOG_ERROR` over `WriteFile` (no stdio) |
| `string.h/.c` | `strcmp`, `AnsiToWide`, the `Format` printf-family |
| `memory.h/.c` | `MemoryZero`/`MemoryCopy` (volatile loops) |
| `system_facts.h/.c` | machine UUID + hostname / user / OS facts |
| `protocol.h` | opcodes, statuses, v5 identity frame, capability mask |
| `wire.h` | tiny little-endian writers (header-only) |

## Study path (how to read the code)

17 translation units, small but dense. A proven reading order — each
step builds on the previous:

1. **`main.c` top to bottom** — the agent's lifecycle and command
   dispatch. Everything else serves this file.
2. **`transport.c`** — how a WebSocket message is sent and how
   fragments are assembled into one message.
3. **`winhttp_api.c`** — the vtable pattern + the `LdrLoadDll`
   bootstrap. The heart of the dependency removal.
4. **`peb.c` + `system.c`** — the PEB walk and the PE export resolver
   the step above relies on.
5. **`entry.c`** — what the C runtime normally does before `main`:
   .bss zeroing, argv marshalling, the stack probe.
6. **`memory.c` / `string.c`** — why plain loops become CRT calls at
   `-O2`, and what `volatile` does about it.
7. **`shell.c`** — pipes, process creation, handle inheritance; the
   most "normal Win32" file in the tree.

Two companion documents live in `notes/` (kept out of the repository
by choice — the author's study notes):
- **COMPILER-PIPELINE.md** — how C becomes an exe: preprocessor,
  compiler (the optimizer's silent substitutions), assembler symbols,
  the linker's puzzle, the loader, the CRT startup. Every error this
  project hit, mapped to its pipeline stage.
- **AGENT-INTERNALS.md** — the mechanisms in depth, plus the history
  of the techniques (PEB walking, hash resolution) from the shellcode
  era to today.

## CI / Releases

- **build.yml** — on every push/PR: compiles the dependency-free
  flavor with llvm-mingw and **fails the build if the import table is
  not empty** (the whole point, gated). On pushes to `main` it also
  republishes the rolling `preview` pre-release (`windows-x86_64.exe`);
- **release.yml** — on a `v*` tag: the same gated build published as a
  stable GitHub Release. Release binaries bake `-DID_BUILD_NUMBER` /
  `-DAGENT_COMMIT_HASH` (local builds fall back to in-code defaults).

## Honest limitations

- **Static surface only.** Removing imports cleans the static import
  table; winhttp/kernel32 still load at runtime and appear in the
  process module list. This is a study of the loading mechanism, not
  an evasion toolkit.
- **x86_64 only** (see Build).
- **Windows 8+** (the WinHTTP WebSocket API).
- The identity frame's breed id is not yet registered in the panel's
  breed table, so the panel currently shows it as "Unknown".
- Two known compiler warnings remain (a dead variable in
  `doubleToStr`; a false-positive `-Warray-bounds` on the `gs:0x60`
  inline asm in `peb.c`) — cosmetic, documented in the notes.

## Related

Part of the Nostdlib workspace: the **relay** (Cloudflare Worker
forwarder) and the **C2 panel** (Blazor WASM operator UI) this agent
talks to live in sibling repositories; the flagship
**Position-Independent-Agent** is the same philosophy taken to a full
PIC implementation (a `.text`-only image, hand-rolled TLS 1.3).

## License & responsible use

Authorized security research and education only. See `LICENSE` and
`RESPONSIBLE_USE.md` in the workspace root for scope and legal
boundaries.
