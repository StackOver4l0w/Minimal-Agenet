#include "types.h"

/// Seed of the module-name hash. A compile-time constant (NOT __DATE__):
/// precomputed hash constants must be reproducible offline by the hash
/// generator, and a date-derived seed silently changed every value on
/// each rebuild. CI may override per build (-DAPI_HASH_SEED=...) to
/// break static AV signatures (OPSEC stage); 5381 is the canonical djb2.
#ifndef API_HASH_SEED
#define API_HASH_SEED 5381ULL
#endif

UINT64 Hash(const WCHAR* str);