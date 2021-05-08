#pragma once
#include <stdint.h>

typedef struct {
    uint32_t h[5];

    uint8_t chunk[64];
    uint64_t length;
} sha1_ctx;

void sha1_init(sha1_ctx *ctx);
void sha1_append(sha1_ctx *ctx, const void *buf, uint64_t len);
void sha1_final(sha1_ctx *ctx);
// out is treated as an 20byte array to which the digest will be copied
void sha1_digest(const sha1_ctx *ctx, void *out);
