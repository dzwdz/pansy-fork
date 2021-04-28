#include <bignum/bignum.h>
#include <stdbool.h>
#include <stdlib.h>

bool test_bignum() {
	bignum *a = bignum_new(64);
	bignum *b = bignum_new(64);
	bignum *c = bignum_new(64);
	bignum *d = bignum_new(64);

	bignum_fromhex(a, "1111000022220000333300004444000055550000dEaDbAbE");
	bignum_print(a);

	bignum_fromhex(b, "1010123412341234123412341234123412341234100c0cc5");
	bignum_print(b);

	bignum_sub(b, a, b);
	bignum_print(b);

	bignum_div(a, b, c, d);
	puts("---");
	bignum_print(c);
	bignum_print(d);
	
	bignum_mul(b, a, a);
	bignum_print(b);

	free(a);
	free(b);
	free(c);
	free(d);
	return true;
}
