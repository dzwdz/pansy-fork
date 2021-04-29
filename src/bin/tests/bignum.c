#include <bignum/bignum.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

bool test_bignum() {
	bignum *a = bignum_new(64);
	bignum *asqr = bignum_new(64);
	bignum *b = bignum_new(64);
	bignum *c = bignum_new(64);
	bignum *d = bignum_new(64);

	bignum_fromhex(a, "1111000022220000333300004444000055550000dEaDbAbE");
	puts("a =");
	bignum_print(a);

	bignum_fromhex(b, "1010123412341234123412341234123412341234100c0cc5");
	puts("b = ");
	bignum_print(b);

	bignum_sub(b, a, b);
	puts("a - b =");
	bignum_print(b);

	bignum_mul(asqr, a, a);
	puts("a * a =");
	bignum_print(asqr);

	bignum_div(asqr, b, c, d);
	puts("a * a / (a - b) =");
	bignum_print(c); // should be 122358e1b41b67a99bce7bebbf2c38b0df58491eb2ce805d9
	puts("remainder:");
	bignum_print(d); // should be 123ab0d0855e7b3f99f82b49f993a86bd76a23afea4ff3
	
	free(a);
	free(asqr);
	free(b);
	free(c);
	free(d);
	return true;
}
