#pragma once
#include "misc.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int fd;
    char *sbuf; // 2048 bytes in size, no guarantees about what's in it
                // i'm just using it to avoid constant mallocs

    // id_exchange
    char *client_id;
} connection;

iter_t read_packet(connection *conn);
uint8_t pop_byte(iter_t *iter);
uint32_t pop_uint32(iter_t *iter);
iter_t pop_string(iter_t *iter);

bool namelist_has(iter_t haystack, const char *needle);

void ssh_fatal(connection *conn);
