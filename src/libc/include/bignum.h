/* WARNING
 * this is EXTREMELY unsafe when used with threads
 * if you're writing anything with threads, be 100% sure that only one thread
 * uses the bignum functions
 * it doesn't matter if another thread would be operating on completely different
 * data - currently, everything uses a shared buffer
 *
 * why? laziness
 * if you need to use those in multiple threads, or if anything breaks, reach
 * out to me
 *     ~dzwdz
 */
#pragma once
#include <stdint.h>

typedef struct {
    uint16_t length;
    uint64_t digits[];
} bignum;

bignum* BN_new(uint16_t size);
void BN_zeroout(bignum *a);
void BN_fromhex(bignum *target, const char *hex);
void BN_print(const bignum *a);

void BN_mul(bignum *result, const bignum *a, const bignum *b);
void BN_mul_karatsuba(bignum *result, const bignum *a, const bignum *b);
void BN_div(const bignum *dividend, const bignum *divisor,
        bignum *quotient, bignum *remainder);
void BN_add(bignum *result, const bignum *a, const bignum *b);
void BN_sub(bignum *result, const bignum *a, const bignum *b);

void BNR_mul_karatsuba(uint64_t *res, uint16_t reslen,
                              const uint64_t *fac1, uint16_t len1,
                              const uint64_t *fac2, uint16_t len2);

uint16_t BN_order(const bignum *bn);
uint64_t BN_log2(const bignum *bn);

void BN_modexp_timingsafe(bignum *result, const bignum *base,
                              const bignum *power, const bignum *modulus);

// returns -1 if a < b
//          0 if a = b
//          1 if a > b
int8_t BN_compare (const bignum *a, const bignum *b);

static inline uint8_t *BN_byteat(bignum *bn, uint16_t pos) {
    return (uint8_t*)bn->digits + pos;
}

// biased, and also probably vulnerable to timing attacks
void BN_random(const bignum *lower, const bignum *upper, bignum *target);
