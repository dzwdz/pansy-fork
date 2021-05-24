#include <assert.h>
#include <stdbool.h>
#include <bignum.h>
#include <string.h>
#include <unistd.h> // getentropy()
#include <stdlib.h> // malloc & free, should be removed (BNA)

const bignum BN_NULL = {
    .length = 0,
    .digits = NULL
};

// returns sign(a - b)
int8_t BN_compare (const bignum a, const bignum b) {
    if (a.length < b.length) {
        for (int i = a.length; i < b.length; i++) {
            if (b.digits[i] != 0) return -1;
        }
    } else if (b.length < a.length) {
        for (int i = b.length; i < a.length; i++) {
            if (a.digits[i] != 0) return -1;
        }
    }

    for (int i = a.length - 1; i >= 0; i--) {
        if (a.digits[i] == b.digits[i]) continue;
        if (a.digits[i] <  b.digits[i]) return -1;
        else                              return  1;
    }

    return 0;
}


uint16_t BN_order(const bignum bn) {
    for (int i = bn.length - 1; i >= 0; i--) {
        if (bn.digits[i] != 0)
            return i + 1;
    }
    return 0;
}

uint64_t BN_log2(const bignum bn) {
    uint16_t order = BN_order(bn);
    if (order == 0) return 0;

    uint64_t digit = bn.digits[order - 1];
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


void BN_random(const bignum lower, const bignum upper, bignum target) {
    bignum tmp = BN_new(upper.length);

    bignum range = BN_new(upper.length);
    BN_sub(range, upper, lower);

    getentropy(tmp.digits, tmp.length * sizeof(uint64_t));
    BN_div(tmp, range, BN_NULL, target);
    BN_add(target, target, lower);

    BN_free(tmp);
    BN_free(range);
}


inline void __BNR_add_loop_body(uint64_t *res, int i, bool *overflow, 
        uint64_t dA, uint64_t dB) {
    *overflow  = __builtin_add_overflow(*overflow ? 1 : 0, dB, &dB);
    *overflow += __builtin_add_overflow(dA, dB, &res[i]);
}

// sorry for this mess
void BN_add(bignum result, const bignum a, const bignum b) {
    bool overflow = false;
    int i = 0;

    uint16_t min = (a.length < b.length) ? a.length : b.length;
    if (result.length < min) min = result.length;
    for (; i < min; i++) {
        __BNR_add_loop_body(result.digits, i, &overflow,
                a.digits[i], b.digits[i]);
    }

    if (min != a.length) { // a.digits has digits remaining
        min = (a.length < result.length) ? a.length : result.length;
        for (; i < min; i++) {
            __BNR_add_loop_body(result.digits, i, &overflow,
                     a.digits[i], 0);
        }
    } else if (min != b.length) { // b.digits has digits remaining
        min = (b.length < result.length) ? b.length : result.length;
        for (; i < min; i++)
            __BNR_add_loop_body(result.digits, i, &overflow,
                     0, b.digits[i]);
    }

    if (i >= result.length) return;
    if (overflow) {
        result.digits[i] = 1;
        i++;
    }
    memset(&result.digits[i], 0, (result.length - i) * sizeof(uint64_t));
}


inline void __BNR_sub_loop_body(uint64_t *res, int i, bool *overflow, 
        uint64_t dA, uint64_t dB) {
    *overflow  = __builtin_add_overflow(*overflow ? 1 : 0, dB, &dB);
    *overflow += __builtin_sub_overflow(dA, dB, &res[i]);
}

bool BN_sub(bignum result, const bignum a, const bignum b) {
    bool overflow = false;
    int i = 0;

    uint16_t min = (a.length < b.length) ? a.length : b.length;
    if (result.length < min) min = result.length;
    for (; i < min; i++) {
        __BNR_sub_loop_body(result.digits, i, &overflow,
                a.digits[i], b.digits[i]);
    }

    if (min != a.length) { // a.digits has digits remaining
        min = (a.length < result.length) ? a.length : result.length;
        for (; i < min; i++) {
            __BNR_sub_loop_body(result.digits, i, &overflow,
                     a.digits[i], 0);
        }
    } else if (min != b.length) { // b.digits has digits remaining
        min = (b.length < result.length) ? b.length : result.length;
        for (; i < min; i++)
            __BNR_sub_loop_body(result.digits, i, &overflow,
                     0, b.digits[i]);
    }

    if (i >= result.length) return overflow; // TODO false negatives
    memset(&result.digits[i], overflow ? 0xFF : 0, (result.length - i) * sizeof(uint64_t));
    return overflow;
}


void BN_mul(bignum result, const bignum a, const bignum b) {
    BN_zeroout(result);

    for (uint16_t i = 0; i < a.length; i++) {
        if (a.digits[i] == 0) continue;
        
        uint16_t upper = result.length - i;
        if (b.length < upper) upper = b.length;

        uint64_t low = 0, high;
        for (uint16_t j = 0; j < upper; j++) { // potential overflow
            uint16_t pos = j + i;
            uint64_t overflow = __builtin_add_overflow(low, result.digits[pos],
                    &result.digits[pos]);

            __int128 product = (__int128)(a.digits[i]) * (__int128)(b.digits[j]); // TODO shouldn't i use uint128?
            low  = product >> 64;
            high = product;
            low += overflow; // refer to comment below

            overflow = __builtin_add_overflow(high, result.digits[pos], &result.digits[pos]);
            assert(__builtin_add_overflow(overflow, low, &low) == 0);
        }
        { // add the last high byte
            uint16_t pos = i + upper;
            if (pos < result.length)
                BNR_addat(&result.digits[pos], result.length - pos, low);
        }
    }
}

/*  you can safely ignore this comment unless you're working on BN_mul
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
