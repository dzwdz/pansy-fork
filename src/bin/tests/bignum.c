#include <bignum/bignum.h>
#include <stdbool.h>
#include <stdlib.h>

bool test_bignum() {
	bignum *a = bignum_new(64);
	bignum *b = bignum_new(64);

	bignum_fromhex(a, "1111000022220000333300004444000055550000dEaDbAbE");
	bignum_print(a);
	
	bignum_mul(b, a, a);
	bignum_print(b);

	free(a);
	return true;
}
