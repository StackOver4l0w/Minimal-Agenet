# WinHTTP Downloader (+ WebSocket Relay Client)

Two small Windows network programs in C, both built on the
[WinHTTP](https://learn.microsoft.com/windows/win32/winhttp/winhttp-start-page) API:

| Program | Source | What it does |
|---|---|---|
| `downloader.exe` | `main.c` | Downloads a file over HTTP(S) and saves it to disk, streaming the body in chunks. |
| `relay_client.exe` | `relay_client.c` | Receive-first WebSocket client: connects to a relay URL, upgrades to WebSocket, and prints every server message (type, hex dump, text) to the terminal. |

## Requirements

- Windows (the WebSocket client needs **Windows 8+** for the WinHTTP WebSocket API)
- A C compiler with the WinHTTP headers. Tested with **MinGW-w64 gcc 16.1**
  (MSYS2 `ucrt64`).

## Build

```sh
gcc -O2 -s -Wall -Wextra -o downloader.exe main.c -lwinhttp
gcc -O2 -s -Wall -Wextra -o relay_client.exe relay_client.c -lwinhttp
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

A receive-first WebSocket client. The relay server sends the first message
itself, so this client never sends anything: it connects, upgrades the HTTP
connection to WebSocket and listens, printing every received message to the
terminal - the WinHTTP buffer type, a hex dump of the payload, and its
printable text. Nothing is written to files.

The URL is not hardcoded anywhere; it comes only from the command line, so the
same binary works against any WebSocket endpoint. Sample output:

```
[1] Connecting to https://relay.example.com/agent ... connected (HTTP 101 Switching Protocols)
[2] Listening for server messages (nothing is sent)...
[1] Received: type=0 (BINARY_MESSAGE), len=1
    0000  00                                                |.|
    text: "."
```

A one-byte binary message containing `0x00` is the relay's first command
(`hello`); the client flags it explicitly when it sees the payload. Note that
WinHTTP does not expose raw RFC 6455 frame opcodes - the printed "type" is the
WinHTTP buffer type (0 = binary message, 2 = UTF-8 message, 1/3 = fragments,
4 = close).

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
- `relay_client` only connects and listens; it does not implement the relay's
  command protocol (no replies are sent). It is lab tooling for observing the
  server side of a WebSocket channel. Detection note: the connection pattern it
  produces (periodic connect to a single fixed host with a non-browser user
  agent) is trivially visible to network monitoring.
