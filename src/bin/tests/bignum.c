#include <assert.h>
#include <bignum.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

void test_bignum() {
    bignum *a = BN_new(3);
    bignum *asqr = BN_new(6);
    bignum *b = BN_new(3);
    bignum *c = BN_new(8);

    bignum *expected = BN_new(8);

    BN_fromhex(a, "1111000022220000333300004444000055550000dEaDbAbE");
    BN_fromhex(b, "1010123412341234f00d00001234123412341234100c0cc5");

    {
        BN_sub(c, a, b);
        BN_fromhex(expected, "100edcc0fededcb43260000320fedcc4320edcccea1adf9");
        assert(BN_compare(c, expected) == 0);
    }

    {
        BN_mul(c, a, b);
        BN_fromhex(expected,
            "1122246acceef3812008a067868cf7f897bbfa34ca09621f56438211a49116e75274e2dd9ca21167161a18421599c36");
        assert(BN_compare(c, expected) == 0);
    }

    { // b ** b (mod a ** 2)
        BN_mul(asqr, a, a);
        BN_modexp_timingsafe(c, b, b, asqr);
        BN_fromhex(expected,
            "a76f78a1708773f866f651af27d7339136c44aab9d7dafeb6087a698a188ec35b5dfea06337ee845d05355938e77c9");
        assert(BN_compare(c, expected) == 0);
    }

    free(a);
    free(asqr);
    free(b);
    free(c);
    free(expected);
}
