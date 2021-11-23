#ifndef SIPHASH_H
#define SIPHASH_H
#include "mtcp.h"

typedef struct {
	uint64_t key[2];
} siphash_key_t;
uint64_t siphash_1u64(const uint64_t a, const siphash_key_t *key);
uint64_t siphash_2u32(const uint32_t a, const uint32_t b, const siphash_key_t *key);
uint64_t siphash_2u64(const uint64_t first, const uint64_t second, const siphash_key_t *key);
uint64_t siphash_4u32(const uint32_t a, const uint32_t b, const uint32_t c, const uint32_t d, const siphash_key_t *key);
#endif