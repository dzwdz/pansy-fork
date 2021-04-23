#include <stdint.h>

typedef struct {
	uint16_t length;
	uint64_t digits[];
} bignum;

bignum* bignum_new(uint16_t size);
void bignum_fromhex(bignum *target, const char *hex);
void bignum_print(const bignum *a);
void bignum_mul(bignum *result, const bignum *a, const bignum *b);
