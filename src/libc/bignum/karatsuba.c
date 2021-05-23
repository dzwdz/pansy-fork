#define _BN_INTERNAL
#include <bignum.h>
#include <string.h>

// 3 is the minimum to avoid infinite recursion
static uint16_t KARATSUBA_THRESHOLD = 128;

void BN_debug_setkt(uint16_t new) {
    KARATSUBA_THRESHOLD = new;
}


void BNR_mul_karatsuba(uint64_t *res, uint16_t reslen,
                              const uint64_t *fac1, uint16_t len1,
                              const uint64_t *fac2, uint16_t len2) {
    uint16_t min_len = len1,
             max_len = len2;
    if (min_len > max_len) {
        min_len = len2;
        max_len = len1;
    }
    if (min_len <= KARATSUBA_THRESHOLD) {
        BNR_mul_naive(res, reslen, fac1, len1, fac2, len2);
        return;
    }
    uint16_t split = min_len >> 1; // amt of high digits that we take
    BNA_push();

    uint16_t z0l = split * 2,
             z1l = len1 + len2 - split * 2 + 1,
             z2l = len1 + len2 - split * 2,
             d1l = len1 - split + 1,
             d2l = len2 - split + 1;
    uint64_t *z0 = BNA_alloc(z0l * sizeof(uint64_t)),
             *z1 = BNA_alloc(z1l * sizeof(uint64_t)),
             *z2 = BNA_alloc(z2l * sizeof(uint64_t)),
             *d1 = BNA_alloc(d1l * sizeof(uint64_t)),
             *d2 = BNA_alloc(d2l * sizeof(uint64_t));

    BNR_mul_karatsuba(z0,           z0l,
                      fac1,         split,
                      fac2,         split);
    BNR_mul_karatsuba(z2,           z2l,
                      &fac1[split], len1 - split,
                      &fac2[split], len2 - split);
    BNR_add          (d1,           d1l,
                      fac1,         split,
                      &fac1[split], len1 - split);
    BNR_add          (d2,           d2l,
                      fac2,         split,
                      &fac2[split], len2 - split);
    BNR_mul_karatsuba(z1, z1l,
                      d1, d1l,
                      d2, d2l);
    BNR_sub          (z1, z1l,
                      z1, z1l,
                      z0, z0l);
    BNR_sub          (z1, z1l,
                      z1, z1l,
                      z2, z2l);

    BNR_mul_naive(res, reslen, fac1, len1, fac2, len2);

    if (reslen < z0l) {
        memcpy(res, z0, sizeof(uint64_t) * reslen);
        goto finish;
    }

    // TODO BNR_cpy
    memcpy(res, z0, sizeof(uint64_t) * z0l);
    memset(&res[z0l], 0, sizeof(uint64_t) * (reslen - z0l));

    if (reslen < split) // impossible
        goto finish;
    BNR_add(&res[split], reslen - split,
            &res[split], reslen - split,
            z1,  z1l);

    if (reslen < split * 2) // also impossible - left here so i don't forget
        goto finish;        // to check bounds when refactoring
    BNR_add(&res[split * 2], reslen - (split * 2),
            &res[split * 2], reslen - (split * 2),
            z2,       z2l);

    finish:

    BNA_pop();
}

void BN_mul_karatsuba(bignum result, const bignum a, const bignum b) {
    BNR_mul_karatsuba(result.digits, result.length,
                      a.digits, a.length,
                      b.digits, b.length);
}

