#include <immintrin.h>

unsigned long _lrotl(unsigned long a, int shift) {
    return (a << shift) | (a >> (32 - shift));
}

unsigned long _lrotr(unsigned long a, int shift) {
    return (a >> shift) | (a << (32 - shift));
}
