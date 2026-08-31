#include "djb2.h"

UINT64 ct_hash_str_seed(const char *s)
{
	UINT64 h = (UINT64)2166136261u; ///< FNV-1 32-bit offset basis (IETF Draft Section 7.1)
	for (UINT64 i = 0; s[i] != '\0'; ++i)
		h = (h ^ (UINT64)(UINT8)s[i]) * (UINT64)16777619u; ///< FNV-1 32-bit prime (IETF Draft Section 7.1)
	return h;
}

UINT64 Hash(const WCHAR* str){
	UINT64 h = ct_hash_str_seed(__DATE__);

	for (UINT64 i = 0; str[i] != L'\0'; ++i) {
		WCHAR c = str[i];
		if (c >= L'A' && c <= L'Z') {
			c = c - L'A' + L'a';
		}
		h = ((h << 5) + h) + (UINT64)c;
	}

	return h;
}