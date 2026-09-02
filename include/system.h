#pragma once
#include "peb.h"

typedef struct _IMAGE_DOS_HEADER_MIN
{
    UINT16 e_magic;
    UINT16 e_cblp;
    UINT16 e_cp;
    UINT16 e_crlc;
    UINT16 e_cparhdr;
    UINT16 e_minalloc;
    UINT16 e_maxalloc;
    UINT16 e_ss;
    UINT16 e_sp;
    UINT16 e_csum;
    UINT16 e_ip;
    UINT16 e_cs;
    UINT16 e_lfarlc;
    UINT16 e_ovno;
    UINT16 e_res[4];
    UINT16 e_oemid;
    UINT16 e_oeminfo;
    UINT16 e_res2[10];
    UINT32 e_lfanew;
} IMAGE_DOS_HEADER_MIN, *PIMAGE_DOS_HEADER_MIN;

#define IMAGE_DOS_SIGNATURE_MIN 0x5A4D
#define IMAGE_NT_SIGNATURE_MIN 0x00004550
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC_MIN 0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC_MIN 0x20b

#define IMAGE_DIRECTORY_ENTRY_EXPORT_MIN 0
#define IMAGE_EXPORT_DIRECTORY_RVA_32 0x60
#define IMAGE_EXPORT_DIRECTORY_RVA_64 0x70

typedef struct _IMAGE_EXPORT_DIRECTORY_MIN
{
    UINT32 Characteristics;
    UINT32 TimeDateStamp;
    UINT16 MajorVersion;
    UINT16 MinorVersion;
    UINT32 Name;
    UINT32 Base;
    UINT32 NumberOfFunctions;
    UINT32 NumberOfNames;
    UINT32 AddressOfFunctions;
    UINT32 AddressOfNames;
    UINT32 AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY_MIN, *PIMAGE_EXPORT_DIRECTORY_MIN;

PVOID ResolveExportByName(PVOID moduleBase, const CHAR *exportName);
PVOID ResolveFromModuleByName(const WCHAR *moduleName, const CHAR *exportName);
PVOID ResolveExportByHash(PVOID moduleBase, UINT64 exportHash);
PVOID ResolveFromModuleByHash(UINT64 moduleNameHash, UINT64 exportHash);
