#define _BN_INTERNAL
#include <bignum.h>
#include <stdlib.h>
#include <string.h>

// https://youtu.be/iGVZIDQl6m0?t=1231
// i'm ripping this algorithm off a youtube video
// shoutouts to the author
//
// the quotient is optional
void BN_div(const bignum dividend, const bignum divisor,
        bignum quotient, bignum remainder) {
    // TODO replace with knuth

    int dividend_order = BN_order(dividend);
    int divisor_order = BN_order(divisor);

    // assert divisor_order != 0

    BN_zeroout(quotient);

    // is the dividend has less digits than the dividend, we can just skip
    // the whole math
    if (dividend_order < divisor_order) {
        BN_copy(remainder, dividend);
        return;
    }

    // remainder = [divisor_order - 1] most significant digits of dividend
    // the most significant digit of the dividend is at [dividend_order - 1]
    //     if we're taking 1 digit we'd start at [dividend_order - 1], copy 1
    //                     2 digits              [dividend_order - 2], copy 2
    BN_zeroout(remainder);
    memcpy(remainder.digits, &dividend.digits[dividend_order - (divisor_order - 1)], (divisor_order - 1) * sizeof(uint64_t));

    bignum intermediate = BN_new(divisor_order + 2);
    bignum d = BN_new(1);
    bignum multiple = BN_new(divisor_order + 2);

    for (int i = 0; i <= dividend_order - divisor_order; i++) {
        BN_zeroout(intermediate);
        // the least significant digit is the [divisor_order + i]th ms one of
        // the dividend
        intermediate.digits[0] =
            dividend.digits[dividend_order - (divisor_order + i)];
        // then the rest of it is the remainder
        memcpy(&intermediate.digits[1],
               remainder.digits,
               divisor_order * sizeof(uint64_t));

        // binary search for the biggest d for which d * divisor < intermediate
        uint64_t low =  0,
                high = ~0;
        while (low <= high) {
            d.digits[0] = (low >> 1) + (high >> 1);

            BN_mul_karatsuba(multiple, d, divisor);
            int8_t diff = BN_compare(multiple, intermediate);

            if (diff < 0) {
                low = d.digits[0] + 1;
            } else if (diff > 0) {
                high = d.digits[0] - 1;
            } else { break; }

            if (low == ~0ull) break; // look i'm too lazy to think about how to fix the binary search
        }

        // d might be wrong, TODO have a big fat think about this
        d.digits[0] = high;
        if (quotient.length > dividend_order - divisor_order - i)
            quotient.digits  [dividend_order - divisor_order - i] = d.digits[0];

        // remainder = intermediate - d * divisor
        BN_copy(remainder, intermediate);
        BN_mul_karatsuba(multiple, d, divisor);
        BN_sub(remainder, remainder, multiple);
    }

    BN_free(intermediate);
    BN_free(d);
    BN_free(multiple);
}

// slower that regular modexp, but more secure against timing attacks
// https://en.wikipedia.org/wiki/Exponentiation_by_squaring#Montgomery's_ladder_technique
void BN_modexp_timingsafe(bignum result, const bignum base,
                              const bignum power, const bignum modulus) {
    uint16_t order1 = BN_order(base),
             order2 = BN_order(modulus);
    // order2 is the max
    if (order1 < order2) order1 = order2;
    order1  = order1 * order1 + 1;
    order1 *= sizeof(uint64_t);

    bignum x1 = BN_new(order1);
    BN_copy(x1, base);
    bignum x2 = BN_new(order1);
    BN_mul(x2, base, base);

    bignum x3 = BN_new(order1); // temporary var

    uint64_t bits = BN_log2(power);
    for (int i = bits - 2; i >= 0; i--) {
        uint64_t bit = power.digits[i / 64]
                     & (1ull << (i % 64));

        // x3 = x1 * x2
        BN_mul(x3, x1, x2);
        if (bit == 0) {
            // x2 = x2 % modules
            BN_div(x3, modulus, BN_NULL, x2);
            BN_mul_karatsuba(x3, x1, x1);
            // x1 = x1 % modules
            BN_div(x3, modulus, BN_NULL, x1);

        } else {
            // x1 = x1 % modules
            BN_div(x3, modulus, BN_NULL, x1);
            BN_mul_karatsuba(x3, x2, x2);
            // x2 = x2 % modules
            BN_div(x3, modulus, BN_NULL, x2);
        }
    }

    BN_copy(result, x1);

    BN_free(x1);
    BN_free(x2);
    BN_free(x3);
}

