#include <bignum.h>
#include <unistd.h>

#define SIZE 8192*4

int main() {
    bignum a = BN_new(SIZE),
           b = BN_new(SIZE),
           c = BN_new(SIZE);

    getentropy(a.digits, a.length * sizeof(uint64_t));
    getentropy(b.digits, b.length * sizeof(uint64_t));

    BN_mul(c, a, b);
}
