#pragma once
#include "types.h"
#include "wintypes.h"

#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type *)((PCHAR)(address) - (USIZE)(&((type *)0)->field)))
#endif

typedef struct _LIST_ENTRY
{
	struct _LIST_ENTRY *Flink; ///< Pointer to the next entry in the list
	struct _LIST_ENTRY *Blink; ///< Pointer to the previous entry in the list
} LIST_ENTRY, *PLIST_ENTRY;

/**
 * @brief Contains the heads of the three module lists maintained by the loader.
 *
 * @details The PEB_LDR_DATA structure is pointed to by PEB.LoaderData and
 * contains the head entries for the three circular doubly-linked lists of
 * LDR_DATA_TABLE_ENTRY structures. Walking these lists provides access to
 * all loaded modules in the process.
 *
 * @see PEB_LDR_DATA structure
 *      https://learn.microsoft.com/en-us/windows/win32/api/winternl/ns-winternl-peb_ldr_data
 */

/**
 * @brief Contains process startup parameters set by the parent process.
 *
 * @details The RTL_USER_PROCESS_PARAMETERS structure is created by
 * RtlCreateProcessParametersEx and stored in PEB.ProcessParameters. It
 * contains the command line, environment, standard I/O handles, and other
 * startup information for the process.
 *
 * @note This is a minimal subset of the full structure, containing only the
 * fields needed by this runtime.
 *
 * @see RTL_USER_PROCESS_PARAMETERS structure
 *      https://learn.microsoft.com/en-us/windows/win32/api/winternl/ns-winternl-rtl_user_process_parameters
 */

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderModuleList;           ///< Links to previous/next module in load order
	LIST_ENTRY InMemoryOrderModuleList;         ///< Links to previous/next module in memory order
	LIST_ENTRY InInitializationOrderModuleList; ///< Links to previous/next module in initialization order
	PVOID DllBase;                              ///< Base address where the module is loaded in memory
	PVOID EntryPoint;                           ///< Address of the module's entry point (DllMain)
	UINT32 SizeOfImage;                         ///< Size of the module image in bytes
	UNICODE_STRING FullDllName;                 ///< Full path of the module (e.g., "C:\Windows\System32\ntdll.dll")
	UNICODE_STRING BaseDllName;                 ///< Base name of the module (e.g., "ntdll.dll")
	UINT32 Flags;                               ///< Loader flags (LDRP_* constants)
	INT16 LoadCount;                            ///< Reference count for the module
	INT16 TlsIndex;                             ///< Thread Local Storage index, or -1 if none
	LIST_ENTRY HashTableEntry;                  ///< Entry in the loader's hash table for fast lookup
	UINT32 TimeDateStamp;                       ///< PE timestamp from the module's file header
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;


typedef struct _RTL_USER_PROCESS_PARAMETERS
{
	UINT32 MaximumLength;  ///< Maximum size of this structure in bytes
	UINT32 Length;         ///< Actual size of this structure in bytes
	UINT32 Flags;          ///< Parameter flags (e.g., RTL_USER_PROC_PARAMS_NORMALIZED)
	UINT32 DebugFlags;     ///< Debug-related flags
	PVOID ConsoleHandle;   ///< Handle to the process's console window
	UINT32 ConsoleFlags;   ///< Console creation flags
	PVOID StandardInput;   ///< Handle to the standard input device
	PVOID StandardOutput;  ///< Handle to the standard output device
	PVOID StandardError;   ///< Handle to the standard error device
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;



typedef struct _PEB_LDR_DATA
{
	UINT32 Length;                                  ///< Size of this structure in bytes
	UINT32 Initialized;                             ///< TRUE after the loader has finished initialization
	PVOID SsHandle;                                 ///< Handle to the loader's heap (internal use)
	LIST_ENTRY InLoadOrderModuleList;               ///< Head of the load-order module list
	LIST_ENTRY InMemoryOrderModuleList;             ///< Head of the memory-order module list
	LIST_ENTRY InInitializationOrderModuleList;     ///< Head of the initialization-order module list
} PEB_LDR_DATA, *PPEB_LDR_DATA;

/**
 * @brief The Process Environment Block, the top-level per-process user-mode structure.
 *
 * @details The PEB is allocated by the kernel during process creation and is
 * accessible from user mode via the Thread Environment Block (TEB). It
 * contains pointers to the loader data (loaded module lists), process
 * parameters (command line, environment, standard handles), the process
 * heap, and the image base address.
 *
 * @note This is a minimal subset of the full PEB structure, containing only
 * the fields needed by this runtime.
 *
 * @see PEB structure
 *      https://learn.microsoft.com/en-us/windows/win32/api/winternl/ns-winternl-peb
 */
typedef struct _PEB
{
	UINT8 InheritedAddressSpace;                    ///< TRUE if the address space was inherited from the parent
	UINT8 ReadImageFileExecOptions;                 ///< TRUE if image file execution options should be read
	UINT8 BeingDebugged;                            ///< TRUE if the process is being debugged
	UINT8 Spare;                                    ///< Reserved (BitField on newer Windows versions)
	PVOID Mutant;                                   ///< Handle to a mutant object (typically -1)
	PVOID ImageBase;                                ///< Base address of the process's executable image
	PPEB_LDR_DATA LoaderData;                       ///< Pointer to PEB_LDR_DATA containing loaded module lists
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters; ///< Pointer to process startup parameters
	PVOID SubSystemData;                            ///< Subsystem-specific data (e.g., Win32 subsystem)
	PVOID ProcessHeap;                              ///< Handle to the default process heap
} PEB, *PPEB;

PPEB GetCurrentPEB(void);
PVOID GetModuleHandleFromPEB(UINT64 moduleNameHash);