#pragma once
#include <stdint.h>

static __inline uint16_t __bswap_16(uint16_t n) {
    return n << 8 | n >> 8;
}

#define bswap_16(x) __bswap_16(x)
#define bswap_32(x) __builtin_bswap32(x)
#define bswap_64(x) __builtin_bswap64(x)
