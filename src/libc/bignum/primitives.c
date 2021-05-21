#include <assert.h>
#include <stdbool.h>
#include <bignum.h>
#include <string.h>
#include <unistd.h> // getentropy()
#include <stdlib.h> // malloc & free, should be removed (BNA)


// returns sign(a - b)
int8_t BN_compare (const bignum *a, const bignum *b) {
    if (a->length < b->length) {
        for (int i = a->length; i < b->length; i++) {
            if (b->digits[i] != 0) return -1;
        }
    } else if (b->length < a->length) {
        for (int i = b->length; i < a->length; i++) {
            if (a->digits[i] != 0) return -1;
        }
    }

    for (int i = a->length - 1; i >= 0; i--) {
        if (a->digits[i] == b->digits[i]) continue;
        if (a->digits[i] <  b->digits[i]) return -1;
        else                              return  1;
    }

    return 0;
}


uint16_t BN_order(const bignum *bn) {
    for (int i = bn->length - 1; i >= 0; i--) {
        if (bn->digits[i] != 0)
            return i + 1;
    }
    return 0;
}

uint64_t BN_log2(const bignum *bn) {
    uint16_t order = BN_order(bn);
    if (order == 0) return 0;

    uint64_t digit = bn->digits[order - 1];
    uint64_t bits = order * sizeof(uint64_t) * 8;

    // we know that digit must not equal 0
    // TODO add an assert
    uint64_t mask = ~(~0ull >> 1); // most significant bit
    while (!(digit & mask)) {
        bits--;
        digit <<= 1;
    }

    return bits;
}


void BN_random(const bignum *lower, const bignum *upper, bignum *target) {
    bignum *tmp = BN_new(upper->length);

    bignum *range = BN_new(upper->length);
    BN_sub(range, upper, lower);

    getentropy(tmp->digits, tmp->length * sizeof(uint64_t));
    BN_div(tmp, range, NULL, target);
    BN_add(target, target, lower);

    free(tmp);
    free(range);
}


inline void __BNR_add_loop_body(uint64_t *res, int i, bool *overflow, 
        uint64_t dA, uint64_t dB) {
    *overflow  = __builtin_add_overflow(*overflow ? 1 : 0, dB, &dB);
    *overflow += __builtin_add_overflow(dA, dB, &res[i]);
}

// sorry for this mess
void BNR_add(uint64_t *res, uint16_t reslen,
             const uint64_t *num1, uint16_t len1,
             const uint64_t *num2, uint16_t len2) {
    bool overflow = false;
    int i = 0;

    uint16_t min = (len1 < len2) ? len1 : len2;
    if (reslen < min) min = reslen;
    for (; i < min; i++) {
        __BNR_add_loop_body(res, i, &overflow,
                num1[i], num2[i]);
    }

    if (min != len1) { // num1 has digits remaining
        min = (len1 < reslen) ? len1 : reslen;
        for (; i < min; i++) {
            __BNR_add_loop_body(res, i, &overflow,
                     num1[i], 0);
        }
    } else if (min != len2) { // num2 has digits remaining
        min = (len2 < reslen) ? len2 : reslen;
        for (; i < min; i++)
            __BNR_add_loop_body(res, i, &overflow,
                     0, num2[i]);
    }

    if (i >= reslen) return;
    if (overflow) {
        res[i] = 1;
        i++;
    }
    memset(&res[i], 0, (reslen - i) * sizeof(uint64_t));
}

void BN_add(bignum *result, const bignum *a, const bignum *b) {
    BNR_add(result->digits, result->length,
            a->digits, a->length,
            b->digits, b->length);
}


inline void __BNR_sub_loop_body(uint64_t *res, int i, bool *overflow, 
        uint64_t dA, uint64_t dB) {
    *overflow  = __builtin_add_overflow(*overflow ? 1 : 0, dB, &dB);
    *overflow += __builtin_sub_overflow(dA, dB, &res[i]);
}

void BNR_sub(uint64_t *res, uint16_t reslen,
             const uint64_t *num1, uint16_t len1,
             const uint64_t *num2, uint16_t len2) {
    bool overflow = false;
    int i = 0;

    uint16_t min = (len1 < len2) ? len1 : len2;
    if (reslen < min) min = reslen;
    for (; i < min; i++) {
        __BNR_sub_loop_body(res, i, &overflow,
                num1[i], num2[i]);
    }

    if (min != len1) { // num1 has digits remaining
        min = (len1 < reslen) ? len1 : reslen;
        for (; i < min; i++) {
            __BNR_sub_loop_body(res, i, &overflow,
                     num1[i], 0);
        }
    } else if (min != len2) { // num2 has digits remaining
        min = (len2 < reslen) ? len2 : reslen;
        for (; i < min; i++)
            __BNR_sub_loop_body(res, i, &overflow,
                     0, num2[i]);
    }

    if (i >= reslen) return;
    memset(&res[i], overflow ? 0xFF : 0, (reslen - i) * sizeof(uint64_t));
}

void BN_sub(bignum *result, const bignum *a, const bignum *b) {
    BNR_sub(result->digits, result->length,
            a->digits,      a->length,
            b->digits,      b->length);
}


void BNR_mul_naive(uint64_t *res, uint16_t reslen,
                   const uint64_t *fac1, uint16_t len1,
                   const uint64_t *fac2, uint16_t len2) {
    memset(res, 0, reslen * sizeof(uint64_t)); // zeroout

    for (uint16_t i = 0; i < len1; i++) {
        if (fac1[i] == 0) continue;
        
        uint16_t upper = reslen - i;
        if (len2 < upper) upper = len2;

        uint64_t low = 0, high;
        for (uint16_t j = 0; j < upper; j++) { // potential overflow
            uint16_t pos = j + i;
            uint64_t overflow = __builtin_add_overflow(low, res[pos], &res[pos]);

            __int128 product = (__int128)(fac1[i]) * (__int128)(fac2[j]);
            low  = product >> 64;
            high = product;
            low += overflow;

            /*
             *  you can safely ignore this comment, unless you're messing with this function
             *  it's badly written too, so you won't miss out on anything
             *
             *  when multiplying ~0 * ~0
             *  low == ~0 - 1
             *  if adding last low overflowed, overflow == 1
             *  so this low == ~0
             *  thus if this overflows too, low will overflow and the assert will fail
             *  actually, i don't think that it's possible
             *  because high == 1
             *  so res[pos] would have to == ~0
             *  which would not be possible to have when overflowing with just a single add
             */
            overflow = __builtin_add_overflow(high, res[pos], &res[pos]);
            assert(__builtin_add_overflow(overflow, low, &low) == 0);
        }
        { // add the last high byte
            uint16_t pos = i + upper;
            if (pos < reslen)
                BNR_addat(&res[pos], reslen - pos, low);
        }
    }
}

void BN_mul(bignum *result, const bignum *a, const bignum *b) {
    BNR_mul_naive(result->digits, result->length,
                  a->digits,      a->length,
                  b->digits,      b->length);
}

