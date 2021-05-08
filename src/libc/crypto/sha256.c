/*
 * this is boring as fuck
 * just read the wikipedia article on sha256
 * NOTE: this assumes a little endian architecture
 */
#include <byteswap.h>
#include <crypto/sha256.h>
#include <immintrin.h>
#include <string.h>

static const uint32_t ROUND_CONSTS[] =  {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void sha256_init(sha256_ctx *ctx) {
    ctx->h0 = 0x6a09e667;
    ctx->h1 = 0xbb67ae85;
    ctx->h2 = 0x3c6ef372;
    ctx->h3 = 0xa54ff53a;
    ctx->h4 = 0x510e527f;
    ctx->h5 = 0x9b05688c;
    ctx->h6 = 0x1f83d9ab;
    ctx->h7 = 0x5be0cd19;

    ctx->length = 0;
}

static void sha256_step(sha256_ctx *ctx) {
    uint32_t w[64], s0, s1, ch, maj, temp1, temp2;
    memcpy(w, ctx->chunk, 64);

    // we have to fix the endianness
    for (int i = 0; i < 16; i++) {
        w[i] = bswap_32(w[i]);
    }

    for (int i = 16; i < 64; i++) {
        s0 = _lrotr(w[i-15],  7) ^ _lrotr(w[i-15], 18) ^ (w[i-15] >>  3);
        s1 = _lrotr(w[i- 2], 17) ^ _lrotr(w[i- 2], 19) ^ (w[i- 2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }

    uint32_t a = ctx->h0,
             b = ctx->h1,
             c = ctx->h2,
             d = ctx->h3,
             e = ctx->h4,
             f = ctx->h5,
             g = ctx->h6,
             h = ctx->h7;

    for (int i = 0; i < 64; i++) {
        s1 = _lrotr(e, 6) ^ _lrotr(e, 11) ^ _lrotr(e, 25);
        ch = (e & f) ^ (~e & g);
        temp1 = h + s1 + ch + ROUND_CONSTS[i] + w[i];
        s0 = _lrotr(a, 2) ^ _lrotr(a, 13) ^ _lrotr(a, 22);
        maj = (a & b) ^ (a & c) ^ (b & c);
        temp2 = s0 + maj;
 
        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    ctx->h0 += a;
    ctx->h1 += b;
    ctx->h2 += c;
    ctx->h3 += d;
    ctx->h4 += e;
    ctx->h5 += f;
    ctx->h6 += g;
    ctx->h7 += h;
}

void sha256_append(sha256_ctx *ctx, const void *buf, uint64_t len) {
    uint8_t cur_pos = ctx->length & 63;
    ctx->length += len;

    while (cur_pos + len >= 64) {
        uint8_t to_add = 64-cur_pos;
        memcpy(ctx->chunk + cur_pos, buf, to_add);
        sha256_step(ctx);

        buf += to_add;
        len -= to_add;
        cur_pos = 0;
    }

    memcpy(ctx->chunk + cur_pos, buf, len);
}

void sha256_final(sha256_ctx *ctx) {
    uint8_t pos = ctx->length & 63;

    if (pos == 64) {
        sha256_step(ctx);
        memset(ctx->chunk, 0, 64);
        pos = 0;
    }
    ctx->chunk[pos++] = 0x80;
    memset(ctx->chunk + pos, 0, 64 - pos);

    if (pos + 8 > 64) {
        sha256_step(ctx);
        memset(ctx->chunk, 0, 64);
    }

    uint64_t tmp = bswap_64(ctx->length * 8);
    memcpy(ctx->chunk + 64 - 8, &tmp, 8);
    sha256_step(ctx);
}

// out is treated as an 32byte array to which the digest will be copied
void sha256_digest(const sha256_ctx *ctx, void *out) {
    ((uint32_t*)out)[0] = bswap_32(ctx->h0);
    ((uint32_t*)out)[1] = bswap_32(ctx->h1);
    ((uint32_t*)out)[2] = bswap_32(ctx->h2);
    ((uint32_t*)out)[3] = bswap_32(ctx->h3);
    ((uint32_t*)out)[4] = bswap_32(ctx->h4);
    ((uint32_t*)out)[5] = bswap_32(ctx->h5);
    ((uint32_t*)out)[6] = bswap_32(ctx->h6);
    ((uint32_t*)out)[7] = bswap_32(ctx->h7);
}

