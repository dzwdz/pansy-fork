#include <bignum.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

bool test_bignum() {
    bignum *a = bignum_new(8);
    bignum *asqr = bignum_new(8);
    bignum *b = bignum_new(8);
    bignum *c = bignum_new(8);

    /*{
        bignum *a2 = bignum_new(3);
        bignum *b2 = bignum_new(3);
        bignum_fromhex(a2, "0100EDCC0FEDEDCB43260000320FEDCC4320EDCCCEA1ADF9");
        bignum_fromhex(b2, "E951961FAF957DFBE105D49DD93B72C528D5EACD7A6FD034");
        bignum_mul(c, a2, b2);
        bignum_print(c);
        free(a2);
        free(b2);
        return true;
    }*/

    bignum_fromhex(a, "1111000022220000333300004444000055550000dEaDbAbE");
    puts("a =");
    bignum_print(a);

    bignum_fromhex(b, "1010123412341234f00d00001234123412341234100c0cc5");
    puts("b = ");
    bignum_print(b);

    puts("b := a - b =");
    bignum_sub(b, a, b);
    bignum_print(b);

    puts("a * a =");
    bignum_mul(asqr, a, a);
    bignum_print(asqr);

    puts("(a*a) * a [naive] =");
    bignum_mul(c, asqr, a);
    bignum_print(c);

    puts("(a*a) * a [karatsuba] =");
    bignum_mul_karatsuba(c, asqr, a);
    bignum_print(c);

    puts("b ** b (mod a ** 2) =");
    bignum_modexp_timingsafe(a, b, b, asqr);
    bignum_print(a);

    free(a);
    free(asqr);
    free(b);
    free(c);
    return true;
}
