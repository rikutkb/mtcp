#ifndef SIPHASH_H
#define SIPHASH_H
#include "mtcp.h"

typedef struct {
	uint64_t key[2];
} siphash_key_t;
uint64_t siphash_1u64(const uint64_t a, const siphash_key_t *key);

static inline uint64_t siphash_2u32(const uint32_t a, const uint32_t b,
			       const siphash_key_t *key)
{
	return siphash_1u64((uint64_t)b << 32 | a, key);
}
static inline uint64_t siphash_4u32(const uint32_t a, const uint32_t b, const uint_32 c,
			       const uint32_t d, const siphash_key_t *key){
                       	return siphash_2u64((uint64_t)b << 32 | a, (uint64_t)d << 32 | c, key);
                   }

#endif