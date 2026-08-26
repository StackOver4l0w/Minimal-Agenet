typedef  long unsigned int DWORD;
typedef void* HANDLE;
typedef unsigned long long UINT64, *PUINT64, **PPUINT64;
typedef void* PVOID;
typedef unsigned int UINT32, *PUINT32, **PPUINT32;
typedef unsigned short UINT16, *PUINT16, **PPUINT16;
typedef unsigned char UINT8, *PUINT8, **PPUINT8;
typedef signed short INT16, *PINT16;
typedef signed int INT32, *PINT32;
typedef unsigned short WCHAR, *PWCHAR, **PPWCHAR;


#define TRUE 1
#define FALSE 0

#define NO_ERROR 0L  
#define ERROR_SUCCESS 0

#ifndef NULL
#define NULL ((void *)0)
#endif



#if defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
#define COMPILER_MSVC
#elif defined(__GNUC__) 
#define COMPILER_GCC
#elif defined(__clang__)
#define COMPILER_CLANG
#endif


// Check windows
#if defined(COMPILER_MSVC)
	#if defined(_WIN64)
		#define ENVIRONMENT_x86_64
	#else
		#define ENVIRONMENT_I386
	#endif
// Check GCC
#elif defined(COMPILER_GCC) 
	#if defined(__aarch64__) || defined(_M_ARM64)
		#define ENVIRONMENT_ARM64
	#elif defined(__arm__) || defined(_M_ARM)
		#define ENVIRONMENT_ARM32
	#elif defined(__x86_64__) || defined(__amd64__) || defined(_M_X64)
		#define ENVIRONMENT_x86_64
	#elif defined(__i386__) || defined(_M_IX86)
		#define ENVIRONMENT_I386
	#else
		#error Unsupported architecture
	#endif

#elif defined(COMPILER_CLANG)
	#if defined(__aarch64__) || defined(_M_ARM64)
		#define ENVIRONMENT_ARM64
	#elif defined(__arm__) || defined(_M_ARM)
		#define ENVIRONMENT_ARM32
	#elif defined(__x86_64__) || defined(__amd64__) || defined(_M_X64)
		#define ENVIRONMENT_x86_64
	#elif defined(__i386__) || defined(_M_IX86)
		#define ENVIRONMENT_I386
	#else
		#error Unsupported architecture
	#endif
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
typedef long LSTATUS;

#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name

DECLARE_HANDLE(HKEY);

typedef DWORD REGSAM;

#if defined(ENVIRONMENT_I386)
typedef unsigned long ULONG_PTR;   // 32-bit
#else
typedef unsigned long long ULONG_PTR;  // 64-bit
#endif

typedef ULONG_PTR SIZE_T, *PSIZE_T, **PPSIZE_T;
typedef long LONG;

#if defined(COMPILER_MSVC)

	#if defined(ENVIRONMENT_I386)

		#define WINAPI __stdcall
		#define WINAPIV __cdecl

	#else

		#define WINAPI
		#define WINAPIV
		
	#endif

#elif defined(COMPILER_GCC) 

	#if defined(ENVIRONMENT_I386)

		#define WINAPI  __stdcall
		#define WINAPIV __cdecl

	#else

		#define WINAPI 
		#define WINAPIV 

	#endif

#elif defined(COMPILER_CLANG)

	#if defined(ENVIRONMENT_I386)

	#define WINAPI  __attribute__((stdcall))
	#define WINAPIV  __attribute__((cdecl))

	#else

		#define WINAPI
		#define WINAPIV
		
	#endif

#endif

#ifndef WINAPI
#define WINAPI
#endif