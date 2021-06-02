#pragma once
#include <stdint.h>
#include <wmmintrin.h>

typedef struct {

} AES_ctx;

/*
 * keysize: one of [256]
 */
void AES_init(AES_ctx *ctx, const uint8_t *key, uint16_t keysize);

void AES_ECB_encrypt(const AES_ctx *ctx, uint8_t *block);
