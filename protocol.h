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

#define CMD_HELLO               0x00
#define CMD_LIST_DIRECTORY      0x01
#define CMD_READ_FILE           0x02
#define CMD_HASH_FILE           0x03
#define CMD_WRITE_SHELL         0x04
#define CMD_READ_SHELL          0x05
#define CMD_GET_DISPLAYS        0x06
#define CMD_GET_SCREENSHOT      0x07
#define CMD_CLOSE_SHELL         0x08
#define CMD_EXIT                0x09
#define CMD_OPEN_SHELL          0x0A

/* ==========================================================================
 * Reply status codes
 * ======================================================================== */

#define STATUS_OK               0
#define STATUS_ERROR            1

/* ==========================================================================
 * The identity frame - the reply to Hello (0x00)
 *
 * Fixed layout, ASCII strings NUL-padded to their field width:
 *
 *   offset  size  field
 *   ------  ----  -------------------------------------------------
 *   0       4     status = 0
 *   4       16    machine UUID, .NET Guid byte order (system_facts.c)
 *   20      256   hostname
 *   276     256   logged-on user name
 *   532     32    CPU architecture ("x64" / "x86")
 *   564     32    platform ("Windows")
 *   596     128   OS version ("10.0.19045")
 *   724     4     build number (display only)
 *   728     9     commit hash (8 chars + NUL, display only)
 *   737     4     API version = 4
 *   741     1     is-64-bit-process flag
 *   742     8     capability mask
 *   ------------------------------------------------------------------------
 *   total: 750 bytes
 * ======================================================================== */

#define IDENTITY_FRAME_SIZE     750
#define ID_HOSTNAME_SIZE        256
#define ID_USERNAME_SIZE        256
#define ID_ARCH_SIZE            32
#define ID_PLATFORM_SIZE        32
#define ID_OS_VERSION_SIZE      128
#define ID_COMMIT_HASH_SIZE     9      /* 8 chars + NUL            */
#define ID_API_VERSION          4      /* v4 = current framing     */

/* Display-only build tag. CI bakes the real one in with
 * -DID_BUILD_NUMBER=<git commit count>; local builds keep the default. */
#ifndef ID_BUILD_NUMBER
#define ID_BUILD_NUMBER         1      /* display-only build tag   */
#endif

/* ==========================================================================
 * Capability mask: 8 bytes, one bit per feature category, LSB first:
 *   bit 0 = FileSystem (ListDirectory/ReadFile/HashFile)
 *   bit 1 = Shell      (Open/Write/Read/CloseShell)  <- implemented here
 *   bit 2 = Display    (GetDisplays/GetScreenshot)
 * The mask gates the panel's UI, not the wire: it may still send any
 * command, so unimplemented opcodes are answered honestly with status 1.
 * ======================================================================== */

#define CAPABILITY_MASK         2

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
