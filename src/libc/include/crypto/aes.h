#pragma once
#include <stdint.h>
#include <wmmintrin.h>

typedef struct {
    __m128i key_schedule[15];
    uint8_t rounds;
} AES_ctx;

/*
 * keysize: one of [256]
 */
void AES_init(AES_ctx *ctx, const uint8_t *key, uint16_t keysize);

__m128i AES_encrypt_block(const AES_ctx *ctx, __m128i block);
