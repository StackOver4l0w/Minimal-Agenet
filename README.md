# WinHTTP Downloader (+ WebSocket Echo Client)

Two small Windows network programs in C, both built on the
[WinHTTP](https://learn.microsoft.com/windows/win32/winhttp/winhttp-start-page) API:

| Program | Source | What it does |
|---|---|---|
| `downloader.exe` | `main.c` | Downloads a file over HTTP(S) and saves it to disk, streaming the body in chunks. |
| `ws_echo.exe` | `ws_echo.c` | WebSocket echo client: connects to `wss://echo.websocket.org`, sends a message, reads the echo back. |

## Requirements

- Windows (the WebSocket client needs **Windows 8+** for the WinHTTP WebSocket API)
- A C compiler with the WinHTTP headers. Tested with **MinGW-w64 gcc 16.1**
  (MSYS2 `ucrt64`).

## Build

```sh
gcc -O2 -s -Wall -Wextra -o downloader.exe main.c -lwinhttp
gcc -O2 -s -Wall -Wextra -o ws_echo.exe ws_echo.c -lwinhttp
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

### ws_echo

```
ws_echo.exe [message]
```

Connects to `wss://echo.websocket.org`, sends the message (or a default one),
and reads frames until the echoed message arrives, then closes cleanly.
Sample output:

```
[1] Connecting to wss://echo.websocket.org/ ... connected (HTTP 101 Switching Protocols)
[2] Sending: "Hello from C WebSocket client!"
[3] Received: "Request served by 4d896d95b55478"
    (server banner - keep reading for the echo...)
[3] Received: "Hello from C WebSocket client!"
    MATCH: the echoed message is identical - client works!
[4] Close frame sent.
```

How the WebSocket upgrade works: an ordinary HTTPS GET request is created,
`WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET` makes WinHTTP add the handshake headers,
`WinHttpSendRequest` + `WinHttpReceiveResponse` exchange the
`101 Switching Protocols` response, and `WinHttpWebSocketCompleteUpgrade`
returns the handle used for `WinHttpWebSocketSend` / `WinHttpWebSocketReceive`.

## Notes / Limitations

- The downloader works only with **direct** HTTP(S) file URLs. Sites that hide
  the real file behind a JavaScript redirect (some "download pages"), or that
  require JavaScript / login (e.g. video platforms), are out of scope — use a
  browser or a dedicated tool (e.g. [yt-dlp](https://github.com/yt-dlp/yt-dlp))
  for those.
- Output filenames are expected to be ASCII in this version.
- `ws_echo` talks to `echo.websocket.org`, which sends a greeting banner before
  the echo; the client reads frames until it sees its own message back.
