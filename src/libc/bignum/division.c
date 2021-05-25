#define _BN_INTERNAL
#include <assert.h>
#include <bignum.h>
#include <stdlib.h>
#include <string.h>

// Knuth - The art of computer programming, vol. 2
// 4.3.1
//
// the quotient is optional
void BN_div(const bignum dividend, const bignum divisor,
        bignum quotient, bignum remainder) {
    BNA_push();

    bignum u,   // dividend
           v,   // divisor
           amputee, tmp;

    BN_zeroout(quotient);

    uint16_t u_order = BN_order(dividend);
    uint16_t v_order = BN_order(divisor);
    if (u_order < v_order) {
        BN_zeroout(quotient);
        //BN_cpy(remainder, dividend); TODO IMPORTANT
        return;
    }

    /* 
     * D1. Normalize 
     */
    uint64_t d = 1; //0x8000000000000000 / divisor.digits[v_order - 1] * 2;
    while (divisor.digits[v_order - 1] * d < 0x8000000000000000) d <<= 1;
    bignum d_bn = {.length = 1, .digits = &d};

    u = BNA_newBN(BN_order(dividend) + 1);
    v = BNA_newBN(v_order);

    BN_mul(u, dividend, d_bn);
    BN_mul(v, divisor,  d_bn);

    // assert(v.digits[v_order - 1] >= 0x8000000000000000);
    // also it shouldn't overflow, which'd be guaranteed if i just bitshifted
    
    /*
     * D2. Initialize j
     */
    uint64_t j = u_order - v_order;

    tmp = BNA_newBN(v_order + 1);

    /*
     * D3. Calculate q
     */
D3: {
        amputee = BN_amputate(u, j);

        __uint128_t dub = ((__uint128_t) u.digits[j + v_order] << 64)
                        + u.digits[j + v_order - 1];

        __uint128_t q = dub / v.digits[v_order - 1];
        __uint128_t rem =  dub % v.digits[v_order - 1];

        // TODO VULN timing attack
this_will_be_a_loop_dont_worry:
        if (
                ((q >> 64) > 0) ||
            ((q * v.digits[v_order - 2]) > ((rem << 64) + u.digits[j + v_order - 2]))
           )
        {
            rem += v.digits[v_order - 1];
            if (rem >> 64 == 0)
                goto this_will_be_a_loop_dont_worry;
        }

        /*
         * D4. Multiply and subtract
         */
        // not the most efficient way to do this, idc (yet)
        uint64_t q2 = q;
        BN_mul(tmp, (bignum) {.length = 1, .digits = &q2}, v);
        bool overflow = BN_sub(amputee, amputee, tmp);

        /*
         * D5. Test remainder
         */
        if (j < quotient.length) {
            quotient.digits[j] = q2;

            if (overflow) {
                /*
                 * D6. Add back
                 */
                quotient.digits[j]--;
                BN_add(amputee, amputee, v);
            }
        }

         /*
          * 7. Loop on j
          */
        if (j-- != 0) goto D3;
    }

    /*
     * 8. Unnormalize
     */
    if (remainder.length > 0) {
        // not the best way to go about this        
        // but since when do i care about the best way
        BN_div(u, d_bn, remainder, BN_NULL);
    }


    BNA_pop();
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

