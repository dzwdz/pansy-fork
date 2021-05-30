#define _BN_INTERNAL
#define _BN_SELF_TEST
#include <assert.h>
#include <bignum.h>
#include <stdlib.h>
#include <stdio.h> // used in selftests
#include <string.h>

void BN_div_single(const bignum dividend, uint64_t divisor,
                   bignum quotient, bignum remainder);

// Knuth - The art of computer programming, vol. 2
// 4.3.1
void BN_div(const bignum dividend, const bignum divisor,
        bignum _quotient, bignum remainder)
{
    uint16_t u_order = BN_order(dividend);
    uint16_t v_order = BN_order(divisor);
    uint64_t j = u_order - v_order; // amt of quotient digits
    bignum u,   // dividend
           v,   // divisor
           quotient = _quotient, // used for selftests
           amputee, tmp;

    // special cases
    assert(v_order != 0);
    if (v_order == 1) {
        BN_div_single(dividend, divisor.digits[0], quotient, remainder);
        return;
    } else if (u_order < v_order) {
        BN_zeroout(quotient);
        BN_copy(remainder, dividend);
        return;
    }

    // the normal case
    {
        BNA_push();
        BN_zeroout(quotient);

        u = BNA_newBN(u_order + 1);
        v = BNA_newBN(v_order);
        tmp = BNA_newBN(v_order + 1);

#ifdef _BN_SELF_TEST
        quotient = BNA_newBN(j + 1);
        BN_zeroout(quotient);
#endif

        /*
         * (D1) Normalizing the divisor
         * TODO this is shit
         */
        uint64_t d = 1;
        while (divisor.digits[v_order - 1] * d < 0x8000000000000000)
            d <<= 1;
        bignum d_bn = {.length = 1, .digits = &d};

        BN_mul(u, dividend, d_bn);
        BN_mul(v, divisor,  d_bn);
        

        do {
            // (D3) Approximate q
            __uint128_t dub = ((__uint128_t) u.digits[j + v_order] << 64)
                            + u.digits[j + v_order - 1];

            __uint128_t qq = dub / v.digits[v_order - 1];
            __uint128_t rem =  dub % v.digits[v_order - 1];

            /* yes, this is a mess. if you want to learn what's going on here,
             * just read the original algorithm */
            while ((qq >> 64) > 0 ||
                  (qq * v.digits[v_order - 2]) >
                    ((rem << 64) + u.digits[j + v_order - 2]))
            {
                qq--;
                rem += v.digits[v_order - 1];
                if (rem >> 64 != 0) break;
            }


            // (D4) Multiply and subtract
            amputee = BN_amputate(u, j);

            uint64_t q = qq; // casting the uint128 to uint64
            BN_mul(tmp, v, (bignum) {.length = 1, .digits = &q});
            bool overflow = BN_sub(amputee, amputee, tmp);


            // (D5 + D6) check if q was too big
            if (overflow) {
                BN_add(amputee, amputee, v);
                q--;
            }

            // save the final q
            if (j < quotient.length)
                quotient.digits[j] = q;
        } while (j-- != 0);


        // (D8) unnormalize
        if (remainder.length > 0)
            BN_div_single(u, d, remainder, BN_NULL);

#ifdef _BN_SELF_TEST
        if (remainder.length > 0) {
            BN_mul(u, quotient, divisor);
            BN_add(u, u, remainder);
            if (BN_compare(u, dividend) != 0) {
                printf("\n%d\tSELFTEST FAILED\n", BN_compare(u, dividend));
                puts("got:");
                BN_print(BN_strip(u));
                puts("dividend:");
                BN_print(BN_strip(dividend));
                puts("divisor:");
                BN_print(BN_strip(divisor));
                puts("quotient:");
                BN_print(BN_strip(quotient));
                puts("remainder:");
                BN_print(BN_strip(remainder));
                assert(false);
            }
        }
        BN_copy(_quotient, quotient);
#endif
        BNA_pop();
    }
}

// divides by a single digit
// it's just a copy-pasted and simplified version of BN_div
void BN_div_single(const bignum dividend, uint64_t divisor,
                   bignum quotient, bignum remainder)
{
    uint16_t u_order = BN_order(dividend);
    uint64_t j = u_order - 1;
    bignum u, amputee, tmp;

    BN_zeroout(quotient);
    if (u_order == 0) {
        BN_zeroout(remainder);
        return;
    }

    {
        BNA_push();

        tmp = BNA_newBN(2);
        u = BNA_newBN(u_order + 1);
        BN_copy(u, dividend);

        do {
            amputee = BN_amputate(u, j);

            __uint128_t dub = ((__uint128_t) u.digits[j + 1] << 64)
                            + u.digits[j];
            uint64_t q = dub / divisor;

            BN_mul(tmp, (bignum) {.length = 1, .digits = &q},
                        (bignum) {.length = 1, .digits = &divisor});

            bool overflow = BN_sub(amputee, amputee, tmp);
            assert(!overflow);

            if (j < quotient.length)
                quotient.digits[j] = q;
        } while (j-- != 0);

        BN_copy(remainder, u);
        BNA_pop();
    }
}

// slower that regular modexp, but more secure against timing attacks
// https://en.wikipedia.org/wiki/Exponentiation_by_squaring#Montgomery's_ladder_technique
void BN_modexp_timingsafe(bignum result, const bignum base,
                              const bignum power, const bignum modulus) {
    uint16_t order1 = BN_order(base),
             order2 = BN_order(modulus);
    // order2 is the max
    if (order1 < order2) order1 = order2;
    order1  = order1 * order1 + 1;
    order1 *= sizeof(uint64_t);

    bignum x1 = BN_new(order1);
    BN_copy(x1, base);
    bignum x2 = BN_new(order1);
    BN_mul(x2, base, base);

    bignum x3 = BN_new(order1); // temporary var

    /*
     * VULN karatsuba is probably vulnerable to timing attacks
     *
     * also, it has the exact same performance here as just calling BN_mul with
     * stripped arguments
     *
     * this could change once you disable optimizations
     */
    uint64_t bits = BN_log2(power);
    for (int i = bits - 2; i >= 0; i--) {
        uint64_t bit = power.digits[i / 64]
                     & (1ull << (i % 64));

        // x3 = x1 * x2
        BN_mul(x3, x1, x2);
        if (bit == 0) {
            // x2 = x2 % modules
            BN_div(x3, modulus, BN_NULL, x2);
            BN_mul_karatsuba(x3, x1, x1);
            // x1 = x1 % modules
            BN_div(x3, modulus, BN_NULL, x1);

        } else {
            // x1 = x1 % modules
            BN_div(x3, modulus, BN_NULL, x1);
            BN_mul_karatsuba(x3, x2, x2);
            // x2 = x2 % modules
            BN_div(x3, modulus, BN_NULL, x2);
        }
    }

    BN_copy(result, x1);

    BN_free(x1);
    BN_free(x2);
    BN_free(x3);
}

