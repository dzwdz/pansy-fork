#include "conn.h"
#include <arpa/inet.h>
#include <bignum.h>
#include <crypto/aes.h>
#include <crypto/hmac.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// reads a single packet into conn.sbuf
// RFC 4253 / 6.
iter_t read_packet(connection *conn) {
    ssize_t bytes = recv(conn->fd, conn->sbuf, SBUF_SIZE, 0);
    if (bytes < 5) ssh_fatal(conn);

    if (conn->using_aes) {
        AES_SDCTR_xcrypt(&conn->aes_c2s, conn->sbuf, 16);
    }

    // the packet size is at the beginning of the buffer
    uint32_t packet_l = ntohl(*(uint32_t*)conn->sbuf);
    // and the padding size is right after it
    uint8_t padding_l = conn->sbuf[4];
    printf("%d - %d, total %d\n", packet_l, padding_l, bytes);

    if (bytes < packet_l + 4 + (conn->using_mac ? 32:0)) {
        puts("fix me you dumbass"); // TODO
        ssh_fatal(conn);
    }

    if (conn->using_aes && packet_l > 16) {
        AES_SDCTR_xcrypt(&conn->aes_c2s, conn->sbuf + 16, packet_l + 4 - 16);
    }

    if (conn->using_mac) {
        uint32_t seqn = htonl(conn->seq_c2s);
        char mac[32];
        HMAC_SHA256_prefix(conn->mac_c2s, 32, &seqn, 4, conn->sbuf, packet_l + 4, mac);
        if (memcmp(conn->sbuf + packet_l + 4, mac, 32) != 0)
            ssh_fatal(conn);
    }

    conn->seq_c2s++;

    iter_t iter;
    iter.base = conn->sbuf + 5;
    iter.max = packet_l - padding_l - 1; // TODO check for overflow
    iter.pos = 0;
    return iter;
}

// starts preparing a packet
iter_t start_packet(connection *conn) {
    iter_t iter;
    iter.base = conn->sbuf + 5;
    iter.max = SBUF_SIZE - 5 - 16;
    iter.pos = 0;
    return iter;
}

// makes some assumptions about the packet iterator
// so it must be used with the one returned by start_packet
void send_packet(connection *conn, iter_t packet) {
    // assert packet.base == conn->sbuf + 5;
    uint8_t  padding_l = 0x1B - (packet.pos & 0xf);
    uint32_t packet_l  = packet.pos + padding_l + 1;

    // make the iterator span the whole packet
    packet.base -= 5;
    packet.pos += 5 + padding_l;
    packet.max = SBUF_SIZE;

    int og_pos = packet.pos; // dumb hack
    packet.pos = 0;
    push_uint32(&packet, packet_l);
    push_byte(&packet, padding_l);
    packet.pos = og_pos; // useless

    send(conn->fd, packet.base, og_pos, 0);
}


// RFC 4251 / 5.
uint8_t pop_byte(iter_t *iter) {
    if ((iter->pos += 1) > iter->max) exit(1); // TODO wtf
    return iter->base[iter->pos - 1];
}

uint32_t pop_uint32(iter_t *iter) {
    if ((iter->pos += 4) > iter->max) exit(1);
    return ntohl(*(uint32_t*)&iter->base[iter->pos - 4]);
}

iter_t pop_string(iter_t *iter) {
    iter_t str;
    str.max = pop_uint32(iter);
    str.base = iter->base + iter->pos;
    str.pos = 0;
    iter->pos += str.max;
    return str;
}

// crashes when the client sends a mpint that's too large to fit in the target
void pop_bignum(iter_t *iter, bignum target) {
    // TODO negatives
    uint32_t len = pop_uint32(iter);
    if ((len > target.length * sizeof(uint64_t))
     || (iter->pos + len > iter->max)) exit(1);

    // a mess
    // TODO BN_frombytes
    BN_zeroout(target);
    for (uint32_t i = 0; i < len; i++) {
        int reverse = len - i - 1;
        target.digits[reverse / 8]
            |= ((uint64_t)(iter->base[iter->pos + i]  & 0xff) << 8 * (reverse % 8));
    }

    iter->pos += len;
}


void push_byte(iter_t *iter, uint8_t val) {
    iter->base[iter->pos] = val;
    if ((iter->pos += 1) > iter->max) exit(1);
}

void push_uint32(iter_t *iter, uint32_t val) {
    val = htonl(val);
    memcpy(iter->base + iter->pos, &val, 4);
    if ((iter->pos += 4) > iter->max) exit(1);
}

void push_string(iter_t *iter, void *buf, uint32_t size) {
    push_uint32(iter, size);
    memcpy(iter->base + iter->pos, buf, size);
    if ((iter->pos += size) > iter->max) exit(1);
}

void push_iter(iter_t *target, iter_t val) {
    // this is getting meta
    push_string(target, val.base, val.max);
}

// doesn't include the null byte
void push_cstring(iter_t *iter, const char *str) {
    uint32_t size = strlen(str);;
    push_uint32(iter, size);
    memcpy(iter->base + iter->pos, str, size);
    if ((iter->pos += size) > iter->max) exit(1);
}

#pragma GCC diagnostic push
// i pass the bignum to the BN_byteat function, which returns a pointer
// thus that function can't have a const argument
// the only thing that i do with that pointer is reading it [at the two marked
// places], so it's safe
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

void push_bignum(iter_t *iter, const bignum bn) {
    int log2 = BN_log2(bn);
    int bytes = (log2 >> 3) + ((log2 & 0b111) ? 1 : 0);
    // TODO support negative bignums
    bool extended = 0x80 & *BN_byteat(bn, bytes - 1); // mark
    
    int len = bytes;
    if (extended) len++;

    push_uint32(iter, len);

    if (extended) {
        push_byte(iter, 0);
        len--;
    }
    while (len--) {
        push_byte(iter, *BN_byteat(bn, len)); // mark
    }
}
#pragma GCC diagnostic pop


// a dumb helper function
bool namelist_has(iter_t haystack, const char *needle) {
    int needlen = strlen(needle); // sorry not sorry
    int i = 0;
    int i_max = haystack.max - needlen;
    if (0 > i_max) return false; // the haystack is too small to contain the needle

    for (;;) {
        for (int j = 0; j < needlen; j++) {
            if (haystack.base[i + j] != needle[j]) {
                i += j; // we won't need to search that area
                        // the needle will never have a comma
                goto next_comma;
            }
        }
        if (i == i_max || 
            haystack.base[i + needlen] == ',') {
            // we've found a match and it isn't longer than the needle
            return true;
        }

next_comma:
        while (haystack.base[i++] != ',') {
            if (i >= i_max)
                return false; // we've reached the end of the name list
        }
        // we've found the comma and we're right after it
    }
}

void ssh_fatal(connection *conn) {
    puts("closing connection due to some fatal error");
    close(conn->fd);
    free(conn->sbuf);
    exit(1);
}

