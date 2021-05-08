/*
 * this is as boring as the SHA256 implementation, half of the code
 * was just copied over. at least it's slightly cleaner because now
 * i possess the power of hindsight
 *
 * NOTE: this assumes a little endian architecture
 */
#include <byteswap.h>
#include <crypto/sha1.h>
#include <immintrin.h>
#include <string.h>

void sha1_init(sha1_ctx *ctx) {
    ctx->h[0] = 0x67452301;
    ctx->h[1] = 0xEFCDAB89;
    ctx->h[2] = 0x98BADCFE;
    ctx->h[3] = 0x10325476;
    ctx->h[4] = 0xC3D2E1F0;

    ctx->length = 0;
}

static void sha1_step(sha1_ctx *ctx) {
    uint32_t w[80], f;
    memcpy(w, ctx->chunk, 64);

    // we have to fix the endianness
    for (int i = 0; i < 16; i++) {
        w[i] = bswap_32(w[i]);
    }

    for (int i = 16; i < 80; i++) {
        w[i] = _lrotl((w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]), 1);
    }

    uint32_t a = ctx->h[0],
             b = ctx->h[1],
             c = ctx->h[2],
             d = ctx->h[3],
             e = ctx->h[4];

    for (int i = 0; i < 80; i++) {
        if (i < 20) {
            f  = (b & c) | (~b & d);
            f += 0x5A827999;
        } else if (i < 40) {
            f  = b ^ c ^ d;
            f += 0x6ED9EBA1;
        } else if (i < 60) {
            f  = (b & c) | (b & d) | (c & d);
            f += 0x8F1BBCDC;
        } else {
            f  = b ^ c ^ d;
            f += 0xCA62C1D6;
        }

        f += _lrotl(a, 5) + e + w[i];
        e = d;
        d = c;
        c = _lrotl(b, 30);
        b = a;
        a = f;
    }

    ctx->h[0] += a;
    ctx->h[1] += b; 
    ctx->h[2] += c;
    ctx->h[3] += d;
    ctx->h[4] += e;
}

// same as the sha256 one
void sha1_append(sha1_ctx *ctx, const void *buf, uint64_t len) {
    uint8_t cur_pos = ctx->length & 63;
    ctx->length += len;

    while (cur_pos + len >= 64) {
        uint8_t to_add = 64-cur_pos;
        memcpy(ctx->chunk + cur_pos, buf, to_add);
        sha1_step(ctx);

        buf += to_add;
        len -= to_add;
        cur_pos = 0;
    }

    memcpy(ctx->chunk + cur_pos, buf, len);
}

// same as the sha256 one
void sha1_final(sha1_ctx *ctx) {
    uint8_t pos = ctx->length & 63;

    if (pos == 64) {
        sha1_step(ctx);
        memset(ctx->chunk, 0, 64);
        pos = 0;
    }
    ctx->chunk[pos++] = 0x80;
    memset(ctx->chunk + pos, 0, 64 - pos);

    if (pos + 8 > 64) {
        sha1_step(ctx);
        memset(ctx->chunk, 0, 64);
    }

    uint64_t tmp = bswap_64(ctx->length * 8);
    memcpy(ctx->chunk + 64 - 8, &tmp, 8);
    sha1_step(ctx);
}

// out is treated as an 20byte array to which the digest will be copied
void sha1_digest(const sha1_ctx *ctx, void *out) {
    for (int i = 0; i < 5; i++) {
        ((uint32_t*)out)[i] = bswap_32(ctx->h[i]);
    }
}

