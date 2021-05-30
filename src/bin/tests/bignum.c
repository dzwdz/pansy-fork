#include <assert.h>
#include <bignum.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

void BN_debug_setkt(uint16_t new);

void test_bignum() {
    bignum a = BN_new(8);
    bignum asqr = BN_new(8);
    bignum b = BN_new(8);
    bignum c = BN_new(8);
    bignum d = BN_new(8);
    bignum e = BN_new(8);

    bignum expected = BN_new(8);

    BN_fromhex(a, "1111000022220000333300004444000055550000dEaDbAbE");
    BN_fromhex(b, "1010123412341234f00d00001234123412341234100c0cc5");

    {
        BN_sub(c, a, b);
        BN_fromhex(expected, "100edcc0fededcb43260000320fedcc4320edcccea1adf9");
        assert(BN_compare(c, expected) == 0); // subtraction
    }

    {
        BN_fromhex(c, "1");
        BN_div(a, c, d, e);
        assert(BN_compare(d, a) == 0);
        BN_fromhex(expected, "0");
        assert(BN_compare(e, expected) == 0);
    }

    {
        BN_mul(c, a, b);
        BN_fromhex(expected,
            "1122246acceef3812008a067868cf7f897bbfa34ca09621f56438211a49116e75274e2dd9ca21167161a18421599c36");
        assert(BN_compare(c, expected) == 0); // multiplication
        BN_div(c, b, d, e);
        assert(BN_compare(d, a) == 0); // division (dividend)
        BN_zeroout(expected);
        assert(BN_compare(e, expected) == 0); // division (remainder)
    }

    {
        BN_fromhex(d,
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

        BN_add(c, d, d);
        BN_fromhex(expected,
                "1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFe");
        assert(BN_compare(c, expected) == 0); // addition w/ overflow

        BN_sub(c, c, d);
        assert(BN_compare(c, d) == 0); // subtraction w/ overflow

        BN_div(c, b, d, e);
        BN_fromhex(expected, "feffde04404ba2b55");
        assert(BN_compare(d, expected) == 0); // division (dividend)
        BN_fromhex(expected, "c89a393e1deb7340b14e1893ca2ec4ad38298bc4ab8ab96");
        assert(BN_compare(e, expected) == 0); // division (remainder)
    }

    {
        BN_debug_setkt(3);

        int a_og = a.length,
            b_og = b.length;
        for (int i = BN_order(a); i <= a_og; i++) {
            a.length = i;
            for (int j = BN_order(b); j <= b_og; j++) {
                b.length = j;
                BN_mul_karatsuba(c, a, b);
                BN_fromhex(expected,
                    "1122246acceef3812008a067868cf7f897bbfa34ca09621f56438211a49116e75274e2dd9ca21167161a18421599c36");
                assert(BN_compare(c, expected) == 0); // karatsuba
            }
        }
        a.length = a_og;
        b.length = b_og;
    }

    { // b ** b (mod a ** 2)
        BN_mul(asqr, a, a);
        BN_modexp_timingsafe(c, b, b, asqr);
        BN_fromhex(expected,
            "a76f78a1708773f866f651af27d7339136c44aab9d7dafeb6087a698a188ec35b5dfea06337ee845d05355938e77c9");
        assert(BN_compare(c, expected) == 0); // modexp
    }

    BN_free(a);
    BN_free(asqr);
    BN_free(b);
    BN_free(c);
    BN_free(d);
    BN_free(e);
    BN_free(expected);
}
