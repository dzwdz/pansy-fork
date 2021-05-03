#pragma once
#include <stdint.h>

typedef struct {
    uint8_t *base;
    uint32_t max;
    uint32_t pos;
} iter_t;

// allocates a new buffer, remember to free it!
iter_t iterator_copy(const iter_t *orig);
