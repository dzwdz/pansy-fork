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
 *
 * the naming convention:
 * BN_   functions that accept a bignum*
 * BNR_  functions that accept raw pointers
 */
#pragma once
#include <stdint.h>

typedef struct {
    uint16_t length;
    uint64_t digits[];
} bignum;



/** primitives.c **/

// returns sign(a - b)
int8_t BN_compare (const bignum *a, const bignum *b);

// returns the index of the last nonzero digit + 1
uint16_t BN_order(const bignum *bn);
uint64_t BN_log2(const bignum *bn);

// biased, and also probably vulnerable to timing attacks
void BN_random(const bignum *lower, const bignum *upper, bignum *target);

void BN_add(bignum *result, const bignum *a, const bignum *b);
void BNR_add(uint64_t *res,        uint16_t reslen,
             const uint64_t *num1, uint16_t len1,
             const uint64_t *num2, uint16_t len2);

void BN_sub(bignum *result, const bignum *a, const bignum *b);
void BNR_sub(uint64_t *res, uint16_t reslen,
             const uint64_t *num1, uint16_t len1,
             const uint64_t *num2, uint16_t len2);

void BN_mul(bignum *result, const bignum *a, const bignum *b);
void BNR_mul_naive(uint64_t *res, uint16_t reslen,
                   const uint64_t *fac1, uint16_t len1,
                   const uint64_t *fac2, uint16_t len2);



/** karatsuba.c **/
void BN_mul_karatsuba(bignum *result, const bignum *a, const bignum *b);
void BNR_mul_karatsuba(uint64_t *res, uint16_t reslen,
                       const uint64_t *fac1, uint16_t len1,
                       const uint64_t *fac2, uint16_t len2);



/** division.c **/
void BN_div(const bignum *dividend, const bignum *divisor,
            bignum *quotient, bignum *remainder);
void BN_modexp_timingsafe(bignum *result, const bignum *base,
                          const bignum *power, const bignum *modulus);


/** alloc.c **/
#ifdef _BN_INTERNAL
#   include <stddef.h>
    void *BNA_alloc(size_t size);
    void  BNA_push();
    void  BNA_pop();
#endif


/** bignum.c **/
bignum* BN_new(uint16_t size);
void BN_zeroout(bignum *a);
void BN_fromhex(bignum *target, const char *hex);
void BN_print(const bignum *a);
void BN_copy(bignum *dest, const bignum *src);



/** here, duh **/
static inline uint8_t *BN_byteat(bignum *bn, uint16_t pos) {
    return (uint8_t*)bn->digits + pos;
}

static inline void BNR_addat(uint64_t *target, uint16_t len, uint64_t to_add) {
    int overflow = __builtin_add_overflow(*target, to_add, target);

    while (overflow && --len) {
        target++;
        overflow = __builtin_add_overflow(*target, 1, target);
    }
}

