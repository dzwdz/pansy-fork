#include "bignum.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define KARATSUBA_THRESHOLD 2

void bignum_zeroout(bignum *a) {
    for (int i = 0; i < a->length; i++) {
        a->digits[i] = 0;
    }
}

static void bignum_debugprint(const uint64_t *digits, uint16_t len) {
    bignum *thisisdumb = bignum_new(len);
    memcpy(thisisdumb->digits, digits, len * sizeof(uint64_t));
    bignum_print(thisisdumb);
    free(thisisdumb);
}

// the size is in an amount of uint64_t, each one is 8 bytes
bignum* bignum_new(uint16_t size) {  
    bignum *b = malloc(sizeof(bignum) + size * sizeof(uint64_t));
    b->length = size;
    bignum_zeroout(b); // TODO this crashes with optimizations enabled
    return b;
}

// will be replaced by a function that returns a string later on
// i don't hate this any less than you do
void bignum_print(const bignum *a) {
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
void bignum_fromhex(bignum *target, const char *hex) {
    bignum_zeroout(target);

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

void bignum_copy(bignum *dest, const bignum *src) {
    // todo optimize
    bignum_zeroout(dest);

    // take the minimum of dest->length, src->length
    int to_copy = dest->length;
    if (src->length < to_copy)
        to_copy = src->length;

    memcpy(dest->digits, src->digits, to_copy * sizeof(uint64_t));
}

static inline void addat_internal(uint64_t *target, uint16_t len, uint64_t to_add) {
    int overflow = __builtin_add_overflow(*target, to_add, target);

    while (overflow && --len) {
        target++;
        overflow = __builtin_add_overflow(*target, 1, target);
    }
}

static inline void bignum_addat(bignum *target, uint16_t pos, uint64_t to_add) {
    addat_internal(&target->digits[pos], target->length - pos, to_add);
}

// returns the index of the last nonzero digit + 1
//         x == 0      = 0
//     0 < x < 2**64   = 1
// 2**64 < x < 2**128  = 2
// etc
uint16_t bignum_order(const bignum *bn) {
    for (int i = bn->length - 1; i >= 0; i--) {
        if (bn->digits[i] != 0)
            return i + 1;
    }
    return 0;
}

uint64_t bignum_log2(const bignum *bn) {
    uint16_t order = bignum_order(bn);
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
void bignum_modexp_timingsafe(bignum *result, const bignum *base,
                              const bignum *power, const bignum *modulus) {
    uint16_t order1 = bignum_order(base),
             order2 = bignum_order(modulus);
    // order2 is the max
    if (order1 < order2) order1 = order2;
    order1  = order1 * order1 + 1;
    order1 *= sizeof(uint64_t);

    bignum *x1 = bignum_new(order1);
    bignum_copy(x1, base);
    bignum *x2 = bignum_new(order1);
    bignum_mul(x2, base, base);

    bignum *x3 = bignum_new(order1); // temporary var

    uint64_t bits = bignum_log2(power);
    for (int i = bits - 2; i >= 0; i--) {
        uint64_t bit = power->digits[i / 64]
                     & (1ull << (i % 64));

        // x3 = x1 * x2
        bignum_mul_karatsuba(x3, x1, x2);
        if (bit == 0) {
            // x2 = x2 % modules
            bignum_div(x3, modulus, NULL, x2);
            bignum_mul_karatsuba(x3, x1, x1);
            // x1 = x1 % modules
            bignum_div(x3, modulus, NULL, x1);

        } else {
            // x1 = x1 % modules
            bignum_div(x3, modulus, NULL, x1);
            bignum_mul_karatsuba(x3, x2, x2);
            // x2 = x2 % modules
            bignum_div(x3, modulus, NULL, x2);
        }
    }

    bignum_copy(result, x1);

    free(x1);
    free(x2);
    free(x3);
}

static void add_internal(uint64_t *res, uint16_t reslen,
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

void bignum_add(bignum *result, const bignum *a, const bignum *b) {
    add_internal(result->digits, result->length, a->digits, a->length,
                 b->digits, b->length);
}

static void sub_internal(uint64_t *res, uint16_t reslen,
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

void bignum_sub(bignum *result, const bignum *a, const bignum *b) {
    sub_internal(result->digits, result->length, a->digits, a->length,
                 b->digits, b->length);
}

// returns -1 if a < b
//          0 if a = b
//          1 if a > b
int8_t bignum_compare (const bignum *a, const bignum *b) {
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

static void naivemul_internal(uint64_t *res, uint16_t reslen,
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
            addat_internal(&res[pos], reslen - pos, low);

            if (fac2[j] == 0) {
                low = 0;
                continue;
            }

            __int128 product = (__int128)(fac1[i]) * (__int128)(fac2[j]);
            low  = product >> 64;
            high = product;
            addat_internal(&res[pos], reslen - pos, high);
        }
        { // add the last high byte
            uint16_t pos = i + upper;
            if (pos < reslen)
                addat_internal(&res[pos], reslen - pos, low);
        }
    }
}

void bignum_mul(bignum *result, const bignum *a, const bignum *b) {
    naivemul_internal(result->digits, result->length, a->digits, a->length,
                      b->digits, b->length);
}

static void karatsuba_internal(uint64_t *res, uint16_t reslen,
                               const uint64_t *fac1, uint16_t len1,
                               const uint64_t *fac2, uint16_t len2) {
    bool debug = false;
    if (len1 <= KARATSUBA_THRESHOLD || len2 <= KARATSUBA_THRESHOLD) {
        naivemul_internal(res, reslen, fac1, len1, fac2, len2);
        return;
    }

    uint16_t min = len1;
    uint16_t max = len2;
    if (max < min) {
        min = len2;
        max = len1;
    }
    uint16_t split = min >> 1; // floor(min / 2)

    // those aren't needed
    bignum *lo1 = bignum_new(split + 1), // +1 to account for when hi1 gets added
           *lo2 = bignum_new(split + 1); // see above

    // we store z0 in the result
    bignum *z1 = bignum_new(min + 1),
           *z2 = bignum_new(max + 1);

    if (debug) {
        puts("lo");
        memcpy(lo1->digits, fac1, split * sizeof(uint64_t));
        memcpy(lo2->digits, fac2, split * sizeof(uint64_t));
        bignum_print(lo1);
        bignum_print(lo2);
    }

    karatsuba_internal(res, reslen, fac1, split,
                                    fac2, split); // z0
    add_internal(lo1->digits, split + 1, fac1, split, &fac1[split], len1 - split);
    add_internal(lo2->digits, split + 1, fac2, split, &fac2[split], len2 - split);
    karatsuba_internal(z1->digits, z1->length, lo1->digits, lo1->length,
                                               lo2->digits, lo2->length); // z1
    karatsuba_internal(z2->digits, z1->length, &fac1[split], len1 - split,
                                               &fac2[split], len2 - split); // z2

    if (debug) {
        puts("z0 z1 z2");
        bignum_debugprint(res, reslen);
        bignum_print(z1);
        bignum_print(z2);
    }

    sub_internal(z1->digits, z1->length, z1->digits, z1->length, res, reslen);
    bignum_sub(z1, z1, z2);

    if (debug) {
        puts("z1 after");
        bignum_print(z1);
        printf("(split %d)\n", split);
    }

    for (int i = 0; i < z1->length; i++) {
        uint16_t pos = split + i;
        if (pos >= reslen) break;
        addat_internal(&res[pos], reslen - pos, z1->digits[i]);
    }
    split *= 2;
    for (int i = 0; i < z2->length; i++) {
        uint16_t pos = split + i;
        if (pos >= reslen) break;
        addat_internal(&res[pos], reslen - pos, z2->digits[i]);
    }

    free(lo1);
    free(lo2);
    free(z1);
    free(z2);

    if (debug) {
        bignum *test = bignum_new(reslen);
        naivemul_internal(test->digits, reslen, fac1, len1, fac2, len2);
        for (int i = 0; i < reslen; i++) {
            if (test->digits[i] != res[i]) {
                puts("expected");
                bignum_print(test);
                memcpy(test->digits, res, reslen * sizeof(uint64_t));
                puts("got");
                bignum_print(test);
                puts("via");
                bignum_debugprint(fac1, len1);
                bignum_debugprint(fac2, len2);
                exit(0);
                break;
            }
        }

        free(test);
    }
}

void bignum_mul_karatsuba(bignum *result, const bignum *a, const bignum *b) {
    karatsuba_internal(result->digits, result->length, a->digits, a->length,
                       b->digits, b->length);
}

// https://youtu.be/iGVZIDQl6m0?t=1231
// i'm ripping this algorithm off a youtube video
// shoutouts to the author
//
// the quotient is optional
void bignum_div(const bignum *dividend, const bignum *divisor,
        bignum *quotient, bignum *remainder) {

    int dividend_order = bignum_order(dividend);
    int divisor_order = bignum_order(divisor);

    // assert divisor_order != 0

    if (quotient != NULL)
        bignum_zeroout(quotient);

    // is the dividend has less digits than the dividend, we can just skip
    // the whole math
    if (dividend_order < divisor_order) {
        bignum_copy(remainder, dividend);
        return;
    }

    // remainder = [divisor_order - 1] most significant digits of dividend
    // the most significant digit of the dividend is at [dividend_order - 1]
    //     if we're taking 1 digit we'd start at [dividend_order - 1], copy 1
    //                     2 digits              [dividend_order - 2], copy 2
    bignum_zeroout(remainder);
    memcpy(remainder->digits, &dividend->digits[dividend_order - (divisor_order - 1)], (divisor_order - 1) * sizeof(uint64_t));

    bignum *intermediate = bignum_new(divisor_order + 2);
    bignum *d = bignum_new(1);
    bignum *multiple = bignum_new(divisor_order + 2);

    for (int i = 0; i <= dividend_order - divisor_order; i++) {
        bignum_zeroout(intermediate);
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

            bignum_mul(multiple, d, divisor);
            int8_t diff = bignum_compare(multiple, intermediate);

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
        bignum_copy(remainder, intermediate);
        bignum_mul(multiple, d, divisor);
        bignum_sub(remainder, remainder, multiple);
    }

    free(intermediate);
    free(d);
    free(multiple);
}

void bignum_random(const bignum *lower, const bignum *upper, bignum *target) {
    bignum *tmp = bignum_new(upper->length);

    bignum *range = bignum_new(upper->length);
    bignum_sub(range, upper, lower);

    getentropy(tmp->digits, tmp->length * sizeof(uint64_t));
    bignum_div(tmp, range, NULL, target);
    bignum_add(target, target, lower);

    free(tmp);
    free(range);
}
