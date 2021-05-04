#include <immintrin.h>

unsigned long _lrotr (unsigned long a, int shift) {
    return (a >> shift) | (a << (32 - shift));
}
