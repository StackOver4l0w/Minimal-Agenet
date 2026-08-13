# WinHTTP Downloader

A minimal command-line file downloader for Windows, written in C using the
[WinHTTP](https://learn.microsoft.com/windows/win32/winhttp/winhttp-start-page) API.

It downloads any file from an HTTP(S) URL and saves it to disk, streaming the
response body in chunks so that even large files use very little memory.

## Requirements

- Windows
- A C compiler with the WinHTTP headers. Tested with **MinGW-w64 gcc 16.1**
  (MSYS2 `ucrt64`).

## Build

```sh
gcc -O2 -s -Wall -Wextra -o downloader.exe main.c -lwinhttp
```

## Usage

```
downloader.exe <URL> <FILE>
```

### Examples

```
downloader.exe https://example.com/ page.html
downloader.exe https://www.python.org/static/img/python-logo.png logo.png
```

## Features

- HTTP **and** HTTPS (TLS is handled automatically by WinHTTP).
- Streams the response in 8 KB chunks (low memory use).
- Prints the size and download progress.
- Refuses to save a file on a non-2xx response (e.g. 404 / 500).

## Notes / Limitations

- Works only with **direct** HTTP(S) file URLs. Sites that hide the real file
  behind a JavaScript redirect (some "download pages"), or that require
  JavaScript / login (e.g. video platforms), are out of scope — use a browser
  or a dedicated tool (e.g. [yt-dlp](https://github.com/yt-dlp/yt-dlp)) for those.
- Output filenames are expected to be ASCII in this version.
