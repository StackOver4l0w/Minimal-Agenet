typedef  long unsigned int DWORD;
typedef void* HANDLE;
typedef unsigned long long UINT64, *PUINT64, **PPUINT64;
typedef void* PVOID;
typedef unsigned int UINT32, *PUINT32, **PPUINT32;
typedef unsigned short UINT16, *PUINT16, **PPUINT16;
typedef unsigned char UINT8, *PUINT8, **PPUINT8;
typedef signed short INT16, *PINT16;
typedef unsigned short WCHAR, *PWCHAR, **PPWCHAR;
#ifndef NULL
#define NULL    ((void*)0)
#endif

#if defined(x86) || defined(_M_IX86)
typedef unsigned int USIZE, *PUSIZE, **PPUSIZE;
#else
typedef unsigned long long USIZE, *PUSIZE, **PPUSIZE;
#endif
typedef char CHAR, *PCHAR, **PPCHAR;
typedef PVOID HINTERNET;
typedef int BOOL;
typedef long NTSTATUS;


#define TRUE 1
#define FALSE 0

#define NO_ERROR 0L  