#include "djb2.h"

/* ct_hash_str_seed is gone: it existed only to derive the seed from the
 * __DATE__ string literal, which (a) lived in .rdata and killed the
 * .text-only blob, and (b) made hash values unrepeatable across
 * rebuilds. The seed is now a plain integer constant - see djb2.h. */

UINT64 Hash(const WCHAR* str){
	UINT64 h = API_HASH_SEED;

	for (UINT64 i = 0; str[i] != L'\0'; ++i) {
		WCHAR c = str[i];
		if (c >= L'A' && c <= L'Z') {
			c = c - L'A' + L'a';
		}
		h = ((h << 5) + h) + (UINT64)c;
	}

	return h;
}