# WinHTTP Downloader (+ WebSocket Relay Client)

Two small Windows network programs in C, both built on the
[WinHTTP](https://learn.microsoft.com/windows/win32/winhttp/winhttp-start-page) API:

| Program | Source | What it does |
|---|---|---|
| `downloader.exe` | `main.c` | Downloads a file over HTTP(S) and saves it to disk, streaming the body in chunks. |
| `relay_client.exe` | `relay_client.c` | Minimal information-only agent: connects to a relay URL over WebSocket, identifies itself to the operator panel (750-byte Hello frame) and answers unimplemented commands with status 1. |

## Requirements

- Windows (the WebSocket client needs **Windows 8+** for the WinHTTP WebSocket API)
- A C compiler with the WinHTTP headers. Tested with **MinGW-w64 gcc 16.1**
  (MSYS2 `ucrt64`).

## Build

```sh
gcc -O2 -s -Wall -Wextra -o downloader.exe main.c -lwinhttp
gcc -O2 -s -Wall -Wextra -o relay_client.exe relay_client.c -lwinhttp -ladvapi32
```

## Usage

### downloader

```
downloader.exe <URL> <FILE>
```

Examples:

```
downloader.exe https://example.com/ page.html
downloader.exe https://www.python.org/static/img/python-logo.png logo.png
```

Features:

- HTTP **and** HTTPS (TLS is handled automatically by WinHTTP).
- Streams the response in 8 KB chunks (low memory use).
- Prints the size and download progress.
- Refuses to save a file on a non-2xx response (e.g. 404 / 500).

### relay_client

```
relay_client.exe <URL>
```

A **minimal information-only agent** for the relay protocol. It connects,
upgrades the HTTP connection to WebSocket, and then implements the smallest
agent contract the operator panel understands:

- `0x00 Hello` -> replies with the full 750-byte identity frame: machine UUID
  (from the registry `MachineGuid`, .NET Guid byte order), hostname, logged-on
  user, architecture, platform, OS version, build metadata, API version 4, and
  a **zero capability mask**;
- `0x09 Exit` -> terminates the agent without replying (the only such command);
- any other command -> replies `status = 1` ("not implemented") - the panel
  shows the device information and hides the file-browser / shell / screen UI
  on its own, because the capability mask declares no feature categories.

The panel therefore lists the agent with full identity (host, user, OS, arch)
while nothing beyond identification is implemented - a pure device-information
agent. Sample output:

```
[1] Connecting to https://relay.example.com/agent ... connected (HTTP 101 Switching Protocols)
[2] Agent mode: replying to commands (capability mask = 0)...
[1] Received: type=0 (BINARY_MESSAGE), len=1
    command: 0x00 - Hello (identify yourself)
    payload: (empty)
    0000  00                                                |.|
    text: "."
[+] panel asks: who are you? (Hello)
[+] identity sent (750 bytes: host=DESKTOP-R4ND0M user=user os=10.0.19045 mask=0)
```

Every received command is printed to the terminal decoded (opcode name +
payload), so the exchange is fully readable. Note that WinHTTP does not expose
raw RFC 6455 frame opcodes - the printed "type" is the WinHTTP buffer type
(0 = binary message, 2 = UTF-8 message, 1/3 = fragments, 4 = close).

How the WebSocket upgrade works: an ordinary HTTPS GET request is created,
`WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET` makes WinHTTP add the handshake headers,
`WinHttpSendRequest` + `WinHttpReceiveResponse` exchange the
`101 Switching Protocols` response, and `WinHttpWebSocketCompleteUpgrade`
returns the handle used for `WinHttpWebSocketReceive`.

## Notes / Limitations

- The downloader works only with **direct** HTTP(S) file URLs. Sites that hide
  the real file behind a JavaScript redirect (some "download pages"), or that
  require JavaScript / login (e.g. video platforms), are out of scope — use a
  browser or a dedicated tool (e.g. [yt-dlp](https://github.com/yt-dlp/yt-dlp))
  for those.
- Output filenames are expected to be ASCII in this version.
- `relay_client` implements only the identification part of the relay protocol
  (Hello / Exit / not-implemented status); there is no file, shell, or screen
  functionality - the zero capability mask tells the panel as much. It is lab
  tooling for a course assignment. Detection note: the connection pattern it
  produces (periodic connect to a single fixed host with a non-browser user
  agent) is trivially visible to network monitoring.
