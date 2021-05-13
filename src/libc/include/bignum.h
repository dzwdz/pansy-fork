#pragma once
#include <stdint.h>

typedef struct {
    uint16_t length;
    uint64_t digits[];
} bignum;

bignum* bignum_new(uint16_t size);
void bignum_zeroout(bignum *a);
void bignum_fromhex(bignum *target, const char *hex);
void bignum_print(const bignum *a);

void bignum_mul(bignum *result, const bignum *a, const bignum *b);
void bignum_div(const bignum *dividend, const bignum *divisor,
        bignum *quotient, bignum *remainder);
void bignum_add(bignum *result, const bignum *a, const bignum *b);
void bignum_sub(bignum *result, const bignum *a, const bignum *b);

uint16_t bignum_order(const bignum *bn);
uint64_t bignum_log2(const bignum *bn);

void bignum_modexp_timingsafe(bignum *result, const bignum *base,
                              const bignum *power, const bignum *modulus);

// returns -1 if a < b
//          0 if a = b
//          1 if a > b
int8_t bignum_compare (const bignum *a, const bignum *b);

static inline uint8_t *bignum_byteat(bignum *bn, uint16_t pos) {
    return (uint8_t*)bn->digits + pos;
}

// biased, and also probably vulnerable to timing attacks
void bignum_random(const bignum *lower, const bignum *upper, bignum *target);
