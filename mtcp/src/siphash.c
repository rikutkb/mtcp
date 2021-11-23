
#include "siphash.h"

#define SIPROUND \
	do { \
	v0 += v1; v1 = rol64(v1, 13); v1 ^= v0; v0 = rol64(v0, 32); \
	v2 += v3; v3 = rol64(v3, 16); v3 ^= v2; \
	v0 += v3; v3 = rol64(v3, 21); v3 ^= v0; \
	v2 += v1; v1 = rol64(v1, 17); v1 ^= v2; v2 = rol64(v2, 32); \
	} while (0)

#define PREAMBLE(len) \
	uint64_t v0 = 0x736f6d6570736575ULL; \
	uint64_t v1 = 0x646f72616e646f6dULL; \
	uint64_t v2 = 0x6c7967656e657261ULL; \
	uint64_t v3 = 0x7465646279746573ULL; \
	uint64_t b = ((uint64_t)(len)) << 56; \
	v3 ^= key->key[1]; \
	v2 ^= key->key[0]; \
	v1 ^= key->key[1]; \
	v0 ^= key->key[0];

#define POSTAMBLE \
	v3 ^= b; \
	SIPROUND; \
	SIPROUND; \
	v0 ^= b; \
	v2 ^= 0xff; \
	SIPROUND; \
	SIPROUND; \
	SIPROUND; \
	SIPROUND; \
	return (v0 ^ v1) ^ (v2 ^ v3);
static inline uint64_t rol64(uint64_t word, unsigned int shift)
{
	return (word << (shift & 63)) | (word >> ((-shift) & 63));
}

uint64_t siphash_1u64(const uint64_t first, const siphash_key_t *key)
{
	PREAMBLE(8)
	v3 ^= first;
	SIPROUND;
	SIPROUND;
	v0 ^= first;
	POSTAMBLE
}
uint64_t siphash_2u64(const uint64_t first, const uint64_t second, const siphash_key_t *key)
{
	PREAMBLE(16)
	v3 ^= first;
	SIPROUND;
	SIPROUND;
	v0 ^= first;
	v3 ^= second;
	SIPROUND;
	SIPROUND;
	v0 ^= second;
	POSTAMBLE
}

uint64_t siphash_2u32(const uint32_t a, const uint32_t b, const siphash_key_t *key)
{
	return siphash_1u64((uint64_t)b << 32 | a, key);
}
uint64_t siphash_4u32(const uint32_t a, const uint32_t b, const uint32_t c, const uint32_t d, const siphash_key_t *key){
		return siphash_2u64((uint64_t)b << 32 | a, (uint64_t)d << 32 | c, key);
}