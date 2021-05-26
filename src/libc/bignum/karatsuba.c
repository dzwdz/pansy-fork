#define _BN_INTERNAL
#include <bignum.h>
#include <string.h>

static inline uint16_t max_u16(uint16_t a, uint16_t b) {
    return a > b ? a : b;
}

// 3 is the minimum to avoid infinite recursion
static uint16_t KARATSUBA_THRESHOLD = 8;

void BN_debug_setkt(uint16_t new) {
    KARATSUBA_THRESHOLD = new;
}


void BN_mul_karatsuba(bignum result, bignum a, bignum b) {
    // possible timing attack
    a = BN_strip(a);
    b = BN_strip(b);

    uint16_t min_len = a.length,
             max_len = b.length;
    if (min_len > max_len) {
        min_len = b.length;
        max_len = a.length;
    }
    if (min_len <= KARATSUBA_THRESHOLD) {
        BN_mul(result, a, b);
        return;
    }
    uint16_t split = min_len >> 1; // amt of high digits that we take
    BNA_push();

    // z0 and z2 get reused to store the summed digits (d1 and d2)
    // this saves a bit of memory
    bignum z0 = BNA_newBN(max_u16(split * 2,
                                  a.length - split + 1)),
           z1 = BNA_newBN(a.length + b.length - split * 2 + 1),
           z2 = BNA_newBN(max_u16(a.length + b.length - split * 2,
                                  b.length - split + 1)),
           d1 = /* */z0,// */BNA_newBN(a.length - split + 1),
           d2 = /* */z2;// */BNA_newBN(b.length - split + 1);

    // split up the factors
    bignum low1 = {.digits = a.digits,         .length = split},
           low2 = {.digits = b.digits,         .length = split},
           hi1  = {.digits = &a.digits[split], .length = a.length - split},
           hi2  = {.digits = &b.digits[split], .length = b.length - split};

    BN_add(d1, low1, hi1);
    BN_add(d2, low2, hi2);

    /*
     * BIGGEST PERFORMANCE HIT
     * if you remove the BN_strip calls at the beginning, this will do a lot of
     * multiplications against 0
     */
    BN_mul_karatsuba(z1, d1, d2);

    BN_mul_karatsuba(z0, low1, low2);
    BN_mul_karatsuba(z2, hi1, hi2);
    BN_sub(z1, z1, z0);
    BN_sub(z1, z1, z2);

    if (result.length < z0.length) {
        memcpy(result.digits, z0.digits, sizeof(uint64_t) * result.length);
        goto finish;
    }

    memcpy(result.digits, z0.digits, sizeof(uint64_t) * z0.length);
    memset(&result.digits[z0.length], 0, sizeof(uint64_t) *
            (result.length - z0.length));

    if (result.length < split) // impossible
        goto finish;
    // what a shit variable name
    bignum up = {
        .digits = &result.digits[split],
        .length = result.length - split
    };
    BN_add(up, up, z1);

    if (result.length < split * 2) // also impossible - left here so i don't forget
        goto finish;               // to check bounds when refactoring
    up = (bignum) {
        .digits = &result.digits[split * 2],
        .length = result.length - (split * 2)
    };
    BN_add(up, up, z2);

    finish:
    BNA_pop();
}

