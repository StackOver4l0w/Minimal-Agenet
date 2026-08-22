# WinHTTP WebSocket Relay Client

A small Windows network program in C, built on the
[WinHTTP](https://learn.microsoft.com/windows/win32/winhttp/winhttp-start-page)
API:

| Program | Source | What it does |
|---|---|---|
| `relay_client.exe` | `main.c` | Minimal agent with a remote shell: connects to a relay URL over WebSocket, identifies itself to the operator panel (750-byte Hello frame), and runs panel commands inside a local `cmd.exe` (Shell category). |

## Requirements

- Windows (**Windows 8+** for the WinHTTP WebSocket API)
- A C compiler with the WinHTTP headers. Tested with **MinGW-w64 gcc 16.1**
  (MSYS2 `ucrt64`).

## Build

```sh
gcc -O2 -s -Wall -Wextra -o relay_client.exe main.c \
    transport.c shell.c report.c system_facts.c -lwinhttp -ladvapi32
```

The agent is split into small modules, one topic per header:

| File | Topic |
|---|---|
| `main.c` | `main`: connect, dispatch commands, cleanup |
| `protocol.h` | opcodes, statuses, identity frame, capability mask |
| `wire.h` | tiny little-endian writers (header-only) |
| `transport.h/.c` | the WebSocket pipe: `ws_send` / `ws_receive` |
| `shell.h/.c` | the cmd.exe pool: spawn / read / write / teardown |
| `report.h/.c` | human-facing output: errors, hex dumps, decoders |
| `system_facts.h/.c` | machine UUID + hostname / user / OS facts |

## Usage

```
relay_client.exe <URL>          # quiet mode (default): one line per event
relay_client.exe <URL> -v       # verbose: dump every command's raw bytes
```

A **minimal agent with a remote shell** for the relay protocol. It connects,
upgrades the HTTP connection to WebSocket, and then serves the operator panel:

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
  process - there is no persistence across reconnects. Detection note: the
  connection pattern it produces (periodic connect to a single fixed host with
  a non-browser user agent) is trivially visible to network monitoring.
