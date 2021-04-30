#include <bignum/bignum.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

bool test_bignum() {
	bignum *a = bignum_new(8);
	bignum *asqr = bignum_new(8);
	bignum *b = bignum_new(8);
	bignum *c = bignum_new(8);
	bignum *d = bignum_new(8);

	bignum_fromhex(a, "1111000022220000333300004444000055550000dEaDbAbE");
	puts("a =");
	bignum_print(a);

	bignum_fromhex(b, "1010123412341234123412341234123412341234100c0cc5");
	puts("b = ");
	bignum_print(b);

	bignum_sub(b, a, b);
	puts("b := a - b =");
	bignum_print(b);

	bignum_mul(asqr, a, a);
	puts("a * a =");
	bignum_print(asqr);

	puts("b ** b (mod a ** 2) =");
	bignum_print(b);
	bignum_print(asqr);
	bignum_modexp_timingsafe(a, b, b, asqr);
	bignum_print(a);
	
	free(a);
	free(asqr);
	free(b);
	free(c);
	free(d);
	return true;
}
