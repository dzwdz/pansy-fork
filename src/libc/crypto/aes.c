#include <assert.h>
#include <crypto/aes.h>
#include <wmmintrin.h>

#include <tty.h> // debugging

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
void AES256_key_expansion(const uint8_t *key, uint8_t *expanded) {
    __m128i high, temp2, low;
    __m128i* schedule = (__m128i*) expanded;

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
    __m128i test = _mm_loadu_si128(key);
    hexdump(&test, 16);
    test = _mm_shuffle_epi32(test, 0xaa);
    hexdump(&test, 16);

    assert(keysize == 256);
    char buf[240];
    AES256_key_expansion(key, buf);
    hexdump(buf, 240);
}

void AES_ECB_encrypt(const AES_ctx *ctx, uint8_t *block) {
}
