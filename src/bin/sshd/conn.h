#pragma once
#include "misc.h"
#include <bignum.h>
#include <crypto/aes.h>
#include <stdbool.h>
#include <stdint.h>

#define SBUF_SIZE 16384

enum EncAlgo { ENC_NONE, ENC_AES256 };
enum MacAlgo { MAC_NONE, MAC_HMAC_SHA256};

struct conn_side {
    // algo_exchange, freed in key exchange
    iter_t initial_payload;

    enum EncAlgo enc;
    AES_ctx aes;

    enum MacAlgo mac;
    char mac_key[32];

    uint32_t seq;
};

typedef struct {
    int fd;
    uint8_t *sbuf;

    // id_exchange
    char *client_id;

    struct conn_side c2s, s2c;

    char session_id[32]; // calculated during the first KEX
} connection;

void init_connection(connection *conn, int fd);

iter_t read_packet(connection *conn);
iter_t start_packet(connection *conn);
void send_packet(connection *conn, iter_t packet);

uint8_t pop_byte(iter_t *iter);
uint32_t pop_uint32(iter_t *iter);
iter_t pop_string(iter_t *iter);
void pop_bignum(iter_t *iter, bignum target);

void push_byte(iter_t *iter, uint8_t val);
void push_uint32(iter_t *iter, uint32_t val);
void push_string(iter_t *iter, void *buf, uint32_t size);
void push_cstring(iter_t *iter, const char *str);
void push_iter(iter_t *target, iter_t val);
void push_bignum(iter_t *iter, const bignum bn);

bool namelist_has(iter_t haystack, const char *needle);

void ssh_fatal(connection *conn);
