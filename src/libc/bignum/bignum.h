#include <stdint.h>

typedef struct {
	uint16_t length;
	uint64_t digits[];
} bignum;

bignum* bignum_new(uint16_t size);
void bignum_fromhex(bignum *target, const char *hex);
void bignum_print(const bignum *a);
void bignum_mul(bignum *result, const bignum *a, const bignum *b);
void bignum_div(const bignum *dividend, const bignum *divisor,
		bignum *quotient, bignum *remainder);
void bignum_sub(bignum *result, const bignum *a, const bignum *b);

// returns -1 if a < b
//          0 if a = b
//          1 if a > b
int8_t bignum_compare (const bignum *a, const bignum *b);
