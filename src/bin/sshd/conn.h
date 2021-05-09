#pragma once
#include "misc.h"
#include <bignum.h>
#include <stdbool.h>
#include <stdint.h>

#define SBUF_SIZE 4096

typedef struct {
    int fd;
    uint8_t *sbuf;

    // id_exchange
    char *client_id;
} connection;

iter_t read_packet(connection *conn);
iter_t start_packet(connection *conn);
void send_packet(connection *conn, iter_t packet);

uint8_t pop_byte(iter_t *iter);
uint32_t pop_uint32(iter_t *iter);
iter_t pop_string(iter_t *iter);
void pop_bignum(iter_t *iter, bignum* target);

void push_byte(iter_t *iter, uint8_t val);
void push_uint32(iter_t *iter, uint32_t val);
void push_string(iter_t *iter, void *buf, uint32_t size);
void push_cstring(iter_t *iter, const char *str);
void push_bignum(iter_t *iter, const bignum *bn);

bool namelist_has(iter_t haystack, const char *needle);

void ssh_fatal(connection *conn);
