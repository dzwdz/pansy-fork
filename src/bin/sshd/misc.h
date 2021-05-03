#pragma once
#include <stdint.h>

typedef struct {
    uint8_t *base;
    uint32_t max;
    uint32_t pos;
} iter_t;
