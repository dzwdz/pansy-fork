#include <assert.h>
#include <crypto/aes.h>
#include <smmintrin.h>
#include <wmmintrin.h>

#include <tty.h> // debugging

// TODO move to some header file
inline __m128i bswap_128(__m128i i) {
    return _mm_shuffle_epi8(i,
        _mm_setr_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));
}

inline void keygen256_assist(__m128i *high, __m128i *low, __m128i *assist) {
    // assist is the value returned by _mm_aeskeygenassist
    __m128i shifted;

    { // 1. computing high
        shifted = _mm_slli_si128(*high, 0x4); // a right shift by 4 bytes
        *high = _mm_xor_si128(*high, shifted);
        shifted = _mm_slli_si128(shifted, 0x4);
        *high = _mm_xor_si128(*high, shifted);
        shifted = _mm_slli_si128(shifted, 0x4);
        *high = _mm_xor_si128(*high, shifted);

        /* expand the last 4byte block of assist
         * example:
         *   41 45 53 2D 32 35 36 20 6E 65 65 64 73 20 61 20   AES-256 needs a
         * turns into
         *   73 20 61 20 73 20 61 20 73 20 61 20 73 20 61 20   s a s a s a s a
         *
         * then high gets XORed with that
         */
        *assist = _mm_shuffle_epi32(*assist, 0xff);
        *high = _mm_xor_si128(*high, *assist);
    }

    { // 2. computing low
        shifted = _mm_slli_si128(*low, 0x4);
        *low = _mm_xor_si128(*low, shifted);
        shifted = _mm_slli_si128(shifted, 0x4);
        *low = _mm_xor_si128(*low, shifted);
        shifted = _mm_slli_si128(shifted, 0x4);
        *low = _mm_xor_si128(*low, shifted);

        *assist = _mm_aeskeygenassist_si128(*high, 0x0);
        /* expand the second-to-last 4byte block of assist
         * example:
         *   41 45 53 2D 32 35 36 20 6E 65 65 64 73 20 61 20   AES-256 needs a
         * turns into
         *   6E 65 65 64 6E 65 65 64 6E 65 65 64 6E 65 65 64   needneedneedneed
         *
         * then XOR low with that
         */
        *assist = _mm_shuffle_epi32(*assist, 0xaa);
        *low = _mm_xor_si128(*low, *assist);
    }
}

/*
 * no, _mm_aeskeygenassist can't be put in keygen256_assist
 * gcc requires it to have a constant argument
 */
void AES256_key_expansion(const uint8_t *key, __m128i *schedule) {
    __m128i high, temp2, low;

    high = _mm_loadu_si128((__m128i*) key);
    low  = _mm_loadu_si128((__m128i*)(key + 16));
    schedule[0] = high;
    schedule[1] = low;

    temp2 = _mm_aeskeygenassist_si128(low, 0x01);
    keygen256_assist(&high, &low, &temp2);
    schedule[2] = high;
    schedule[3] = low;

    temp2 = _mm_aeskeygenassist_si128(low, 0x02);
    keygen256_assist(&high, &low, &temp2);
    schedule[4] = high;
    schedule[5] = low;

    temp2 = _mm_aeskeygenassist_si128(low, 0x04);
    keygen256_assist(&high, &low, &temp2);
    schedule[6] = high;
    schedule[7] = low;

    temp2 = _mm_aeskeygenassist_si128(low, 0x08);
    keygen256_assist(&high, &low, &temp2);
    schedule[8] = high;
    schedule[9] = low;

    temp2 = _mm_aeskeygenassist_si128(low, 0x10);
    keygen256_assist(&high, &low, &temp2);
    schedule[10] = high;
    schedule[11] = low;

    temp2 = _mm_aeskeygenassist_si128(low, 0x20);
    keygen256_assist(&high, &low, &temp2);
    schedule[12] = high;
    schedule[13] = low;

    temp2 = _mm_aeskeygenassist_si128(low, 0x40);
    keygen256_assist(&high, &low, &temp2);
    schedule[14] = high;
}

void AES_init(AES_ctx *ctx, const uint8_t *key, uint16_t keysize) {
    assert(keysize == 256);

    ctx->rounds = 14;
    AES256_key_expansion(key, ctx->key_schedule);
}

__m128i AES_encrypt_block(const AES_ctx *ctx, __m128i block) {
    block = _mm_xor_si128(block, ctx->key_schedule[0]);
    for (int i = 1; i < ctx->rounds; i++)
        block = _mm_aesenc_si128(block, ctx->key_schedule[i]);
    block = _mm_aesenclast_si128(block, ctx->key_schedule[ctx->rounds]);
    return block;
}

void AES_SDCTR_set(AES_ctx *ctx, const void *state) {
    _mm_storeu_si128(&ctx->ctr,
            bswap_128(_mm_loadu_si128(state)));
}

void AES_SDCTR_xcrypt(AES_ctx *ctx, void *data, size_t length) {
    __m128i *blocks = data;
    __m128i mask, carry;

    assert(length % 16 == 0);
    length /= 16;

    for (int i = 0; i < length; i++) {
        mask = AES_encrypt_block(ctx, bswap_128(ctx->ctr));

        _mm_storeu_si128(&blocks[i],
                _mm_xor_si128(_mm_loadu_si128(&blocks[i]), mask));

        // incrementing CTR
        const __m128i ONE = _mm_setr_epi32(1, 0, 0, 0);
        ctx->ctr = _mm_add_epi64(ctx->ctr, ONE); // increment the low half

        const __m128i ZERO = _mm_setzero_si128();
        carry = _mm_cmpeq_epi64(ctx->ctr, ZERO); // compare both halves to 0
        carry = _mm_unpacklo_epi64(ZERO, carry); // grab the low half's result and put it on the high half
        ctx->ctr = _mm_sub_epi64(ctx->ctr, carry);
    }
}
