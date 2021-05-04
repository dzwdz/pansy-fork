#pragma once
#include <stdint.h>

typedef struct {
    uint32_t h0, h1, h2, h3, h4, h5, h6, h7;

    uint8_t chunk[64];
    uint64_t length;
} sha256_ctx;

void sha256_init(sha256_ctx *ctx);
void sha256_append(sha256_ctx *ctx, const void *buf, uint64_t len);
void sha256_final(sha256_ctx *ctx);
// out is treated as an 32byte array to which the digest will be copied
void sha256_digest(const sha256_ctx *ctx, void *out);
