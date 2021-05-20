/*
 * the naming convention:
 * BN_   functions that accept a bignum*
 * BNR_  functions that accept raw pointers
 */
#include "bignum.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// 3 is the minimum to avoid infinite recursion
uint16_t KARATSUBA_THRESHOLD = 128;

void BN_debug_setkt(uint16_t new) {
    KARATSUBA_THRESHOLD = new;
}

void *BNA_alloc(size_t size);
void  BNA_push();
void  BNA_pop();

void BN_zeroout(bignum *a) {
    for (int i = 0; i < a->length; i++) {
        a->digits[i] = 0;
    }
}

static void BN_debugprint(const uint64_t *digits, uint16_t len) {
    bignum *thisisdumb = BN_new(len);
    memcpy(thisisdumb->digits, digits, len * sizeof(uint64_t));
    BN_print(thisisdumb);
    free(thisisdumb);
}

// the size is in an amount of uint64_t, each one is 8 bytes
bignum* BN_new(uint16_t size) {  
    bignum *b = malloc(sizeof(bignum) + size * sizeof(uint64_t));
    b->length = size;
    BN_zeroout(b); // TODO this crashes with optimizations enabled
    return b;
}

// will be replaced by a function that returns a string later on
// i don't hate this any less than you do
void BN_print(const bignum *a) {
    for (int i = a->length - 1; i >= 0; i--) {
        int j = sizeof(uint64_t) * 8;
        while (j > 0) {
            j -= 4;
            char c = '0' + ((a->digits[i] >> j) & 0xf);
            if (c > '9') c += 'A' - '9' - 1;
            write(1, &c, 1);
        }
        printf(" ");
    }
    puts("");
}

// will do very weird things if the input string isn't hex
void BN_fromhex(bignum *target, const char *hex) {
    BN_zeroout(target);

    int nibble = 0;
    uint64_t digit;
    for (int i = strlen(hex) - 1; i >= 0; i--) {
        char c = hex[i];
             if ('0' <= c && c <= '9') digit = hex[i] - '0';
        else if ('a' <= c && c <= 'f') digit = hex[i] - 'a' + 0xa;
        else if ('A' <= c && c <= 'F') digit = hex[i] - 'A' + 0xA;
        else digit = 0;

        if (nibble & 1) digit <<= 4;
        int byte = (nibble >> 1) & 0b111;
        digit <<= 8 * byte;

        target->digits[nibble >> 4] |= digit;

        nibble++;
    }
}

void BN_copy(bignum *dest, const bignum *src) {
    // todo optimize
    BN_zeroout(dest);

    // take the minimum of dest->length, src->length
    int to_copy = dest->length;
    if (src->length < to_copy)
        to_copy = src->length;

    memcpy(dest->digits, src->digits, to_copy * sizeof(uint64_t));
}

static inline void BNR_addat(uint64_t *target, uint16_t len, uint64_t to_add) {
    int overflow = __builtin_add_overflow(*target, to_add, target);

    while (overflow && --len) {
        target++;
        overflow = __builtin_add_overflow(*target, 1, target);
    }
}

static inline void BN_addat(bignum *target, uint16_t pos, uint64_t to_add) {
    BNR_addat(&target->digits[pos], target->length - pos, to_add);
}

// returns the index of the last nonzero digit + 1
//         x == 0      = 0
//     0 < x < 2**64   = 1
// 2**64 < x < 2**128  = 2
// etc
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

// slower that regular modexp, but more secure against timing attacks
// https://en.wikipedia.org/wiki/Exponentiation_by_squaring#Montgomery's_ladder_technique
void BN_modexp_timingsafe(bignum *result, const bignum *base,
                              const bignum *power, const bignum *modulus) {
    uint16_t order1 = BN_order(base),
             order2 = BN_order(modulus);
    // order2 is the max
    if (order1 < order2) order1 = order2;
    order1  = order1 * order1 + 1;
    order1 *= sizeof(uint64_t);

    bignum *x1 = BN_new(order1);
    BN_copy(x1, base);
    bignum *x2 = BN_new(order1);
    BN_mul(x2, base, base);

    bignum *x3 = BN_new(order1); // temporary var

    uint64_t bits = BN_log2(power);
    for (int i = bits - 2; i >= 0; i--) {
        uint64_t bit = power->digits[i / 64]
                     & (1ull << (i % 64));

        // x3 = x1 * x2
        BN_mul(x3, x1, x2);
        if (bit == 0) {
            // x2 = x2 % modules
            BN_div(x3, modulus, NULL, x2);
            BN_mul(x3, x1, x1);
            // x1 = x1 % modules
            BN_div(x3, modulus, NULL, x1);

        } else {
            // x1 = x1 % modules
            BN_div(x3, modulus, NULL, x1);
            BN_mul(x3, x2, x2);
            // x2 = x2 % modules
            BN_div(x3, modulus, NULL, x2);
        }
    }

    BN_copy(result, x1);

    free(x1);
    free(x2);
    free(x3);
}

static void BNR_add(uint64_t *res, uint16_t reslen,
                         const uint64_t *num1, uint16_t len1,
                         const uint64_t *num2, uint16_t len2) {
    bool overflow = false;
    uint64_t dA, dB;
    for (int i = 0; i < reslen; i++) {
        dA = (i < len1) ? num1[i] : 0;
        dB = (i < len2) ? num2[i] : 0;

        if (overflow) {
            if (dB == (uint64_t) ~0) { // untested
                res[i] = dA;
                continue;
            }
            dB++;
            overflow = false;
        }
        overflow = __builtin_add_overflow(dA, dB, &res[i]);
    }
}

void BN_add(bignum *result, const bignum *a, const bignum *b) {
    BNR_add(result->digits, result->length, a->digits, a->length,
                 b->digits, b->length);
}

static void BNR_sub(uint64_t *res, uint16_t reslen,
                         const uint64_t *num1, uint16_t len1,
                         const uint64_t *num2, uint16_t len2) {
    bool overflow = false;
    uint64_t dA, dB;
    for (int i = 0; i < reslen; i++) {
        dA = (i < len1) ? num1[i] : 0;
        dB = (i < len2) ? num2[i] : 0;

        if (overflow) {
            if (dB == (uint64_t) ~0) { // untested
                res[i] = dA;
                continue;
            }
            dB++;
            overflow = false;
        }
        overflow = __builtin_sub_overflow(dA, dB, &res[i]);
    }
}

void BN_sub(bignum *result, const bignum *a, const bignum *b) {
    BNR_sub(result->digits, result->length, a->digits, a->length,
                 b->digits, b->length);
}

// returns -1 if a < b
//          0 if a = b
//          1 if a > b
int8_t BN_compare (const bignum *a, const bignum *b) {
    // handle numbers of different sizes
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
                                    else  return  1;
    }

    return 0;
}

static void BNR_mul_naive(uint64_t *res, uint16_t reslen,
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
            BNR_addat(&res[pos], reslen - pos, low);

            if (fac2[j] == 0) {
                low = 0;
                continue;
            }

            __int128 product = (__int128)(fac1[i]) * (__int128)(fac2[j]);
            low  = product >> 64;
            high = product;
            BNR_addat(&res[pos], reslen - pos, high);
        }
        { // add the last high byte
            uint16_t pos = i + upper;
            if (pos < reslen)
                BNR_addat(&res[pos], reslen - pos, low);
        }
    }
}

void BN_mul(bignum *result, const bignum *a, const bignum *b) {
    BNR_mul_naive(result->digits, result->length, a->digits, a->length,
                      b->digits, b->length);
}

void BNR_mul_karatsuba(uint64_t *res, uint16_t reslen,
                              const uint64_t *fac1, uint16_t len1,
                              const uint64_t *fac2, uint16_t len2) {
    uint16_t min_len = len1,
             max_len = len2;
    if (min_len > max_len) {
        min_len = len2;
        max_len = len1;
    }
    if (min_len <= KARATSUBA_THRESHOLD) {
        BNR_mul_naive(res, reslen, fac1, len1, fac2, len2);
        return;
    }
    uint16_t split = min_len >> 1; // amt of high digits that we take
    BNA_push();

    uint16_t z0l = split * 2,
             z1l = len1 + len2 - split * 2 + 1,
             z2l = len1 + len2 - split * 2,
             d1l = len1 - split + 1,
             d2l = len2 - split + 1;
    uint64_t *z0 = BNA_alloc(z0l * sizeof(uint64_t)),
             *z1 = BNA_alloc(z1l * sizeof(uint64_t)),
             *z2 = BNA_alloc(z2l * sizeof(uint64_t)),
             *d1 = BNA_alloc(d1l * sizeof(uint64_t)),
             *d2 = BNA_alloc(d2l * sizeof(uint64_t));

    BNR_mul_karatsuba(z0,           z0l,
                      fac1,         split,
                      fac2,         split);
    BNR_mul_karatsuba(z2,           z2l,
                      &fac1[split], len1 - split,
                      &fac2[split], len2 - split);
    BNR_add          (d1,           d1l,
                      fac1,         split,
                      &fac1[split], len1 - split);
    BNR_add          (d2,           d2l,
                      fac2,         split,
                      &fac2[split], len2 - split);
    BNR_mul_karatsuba(z1, z1l,
                      d1, d1l,
                      d2, d2l);
    BNR_sub          (z1, z1l,
                      z1, z1l,
                      z0, z0l);
    BNR_sub          (z1, z1l,
                      z1, z1l,
                      z2, z2l);

    BNR_mul_naive(res, reslen, fac1, len1, fac2, len2);

    if (reslen < z0l) {
        memcpy(res, z0, sizeof(uint64_t) * reslen);
        goto finish;
    }

    // TODO BNR_cpy
    memcpy(res, z0, sizeof(uint64_t) * z0l);
    memset(&res[z0l], 0, sizeof(uint64_t) * (reslen - z0l));

    if (reslen < split) // impossible
        goto finish;
    BNR_add(&res[split], reslen - split,
            &res[split], reslen - split,
            z1,  z1l);

    if (reslen < split * 2) // also impossible - left here so i don't forget
                            // to check bounds when refactoring
        goto finish;
    BNR_add(&res[split * 2], reslen - (split * 2),
            &res[split * 2], reslen - (split * 2),
            z2,       z2l);

    finish:
    /*
       puts("\n\n");
       BN_debugprint(fac1, len1);
       BN_debugprint(fac2, len2);
       BN_print(d2);
       printf("z (split %d)\n", split);
       BN_print(z0);
       BN_print(z1);
       BN_print(z2);
       puts("=");
       BN_debugprint(res, reslen);
    // */

    BNA_pop();
}

void BN_mul_karatsuba(bignum *result, const bignum *a, const bignum *b) {
    BNR_mul_karatsuba(result->digits, result->length,
                      a->digits, a->length,
                      b->digits, b->length);
}

// https://youtu.be/iGVZIDQl6m0?t=1231
// i'm ripping this algorithm off a youtube video
// shoutouts to the author
//
// the quotient is optional
void BN_div(const bignum *dividend, const bignum *divisor,
        bignum *quotient, bignum *remainder) {

    int dividend_order = BN_order(dividend);
    int divisor_order = BN_order(divisor);

    // assert divisor_order != 0

    if (quotient != NULL)
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
    memcpy(remainder->digits, &dividend->digits[dividend_order - (divisor_order - 1)], (divisor_order - 1) * sizeof(uint64_t));

    bignum *intermediate = BN_new(divisor_order + 2);
    bignum *d = BN_new(1);
    bignum *multiple = BN_new(divisor_order + 2);

    for (int i = 0; i <= dividend_order - divisor_order; i++) {
        BN_zeroout(intermediate);
        // the least significant digit is the [divisor_order + i]th ms one of
        // the dividend
        intermediate->digits[0] =
            dividend->digits[dividend_order - (divisor_order + i)];
        // then the rest of it is the remainder
        memcpy(&intermediate->digits[1],
               remainder->digits,
               divisor_order * sizeof(uint64_t));

        // binary search for the biggest d for which d * divisor < intermediate
        uint64_t low =  0,
                high = ~0;
        while (low <= high) {
            d->digits[0] = (low >> 1) + (high >> 1);

            BN_mul(multiple, d, divisor);
            int8_t diff = BN_compare(multiple, intermediate);

            if (diff < 0) {
                low = d->digits[0] + 1;
            } else if (diff > 0) {
                high = d->digits[0] - 1;
            } else { break; }

            if (low == ~0ull) break; // look i'm too lazy to think about how to fix the binary search
        }

        // d might be wrong, TODO have a big fat think about this
        d->digits[0] = high;
        if (quotient != NULL)
            quotient->digits[dividend_order - divisor_order - i] = d->digits[0];

        // remainder = intermediate - d * divisor
        BN_copy(remainder, intermediate);
        BN_mul_karatsuba(multiple, d, divisor);
        BN_sub(remainder, remainder, multiple);
    }

    free(intermediate);
    free(d);
    free(multiple);
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
