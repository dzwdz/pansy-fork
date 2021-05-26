#include "bignum.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// the size is in an amount of uint64_t, each one is 8 bytes
bignum BN_new(uint16_t size) {  
    bignum b;
    b.digits = malloc(sizeof(bignum) + size * sizeof(uint64_t));
    b.length = size;
    BN_zeroout(b);
    return b;
}

void BN_free(bignum bn) {
    free(bn.digits);
}

void BN_zeroout(bignum a) {
    for (int i = 0; i < a.length; i++) {
        a.digits[i] = 0;
    }
}

void BN_fromhex(bignum target, const char *hex) {
    BN_zeroout(target);

    int nibble = 0;
    uint64_t digit;
    for (int i = strlen(hex) - 1; i >= 0; i--) {
        char c = hex[i];
             if ('0' <= c && c <= '9') digit = hex[i] - '0';
        else if ('a' <= c && c <= 'f') digit = hex[i] - 'a' + 0xa;
        else if ('A' <= c && c <= 'F') digit = hex[i] - 'A' + 0xA;
        else continue;

        if (nibble & 1) digit <<= 4;
        int byte = (nibble >> 1) & 0b111;
        digit <<= 8 * byte;

        target.digits[nibble >> 4] |= digit;

        nibble++;
    }
}

// will be replaced by a function that returns a string later on
// i don't hate this any less than you do
void BN_print(const bignum a) {
    for (int i = a.length - 1; i >= 0; i--) {
        // TODO add uint64_t support to printf
        int j = sizeof(uint64_t) * 8;
        while (j > 0) {
            j -= 4;
            char c = '0' + ((a.digits[i] >> j) & 0xf);
            if (c > '9') c += 'A' - '9' - 1;
            write(1, &c, 1);
        }
//        printf(" ");
    }
    puts("");
}

void BN_copy(bignum dest, const bignum src) {
    // TODO optimize
    BN_zeroout(dest);

    // take the minimum of dest.length, src.length
    int to_copy = dest.length;
    if (src.length < to_copy)
        to_copy = src.length;

    memcpy(dest.digits, src.digits, to_copy * sizeof(uint64_t));
}

