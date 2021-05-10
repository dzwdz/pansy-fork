/*
 * here are the functions that handle the "higher-level" parts of the SSH
 * protocol
 */

#include "conn.h"
#include "crypto.h"
#include "magics.h"
#include "misc.h"
#include "proto.h"
#include <bignum.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

#include <tty.h>

void id_exchange(connection *conn) {
    int bytes = recv(conn->fd, conn->sbuf, SBUF_SIZE, 0);
    if (bytes <= 0) ssh_fatal(conn);

    // TODO this is not spec compliant
    // we find the first line break - it *should* be right after the client id
    int cid_len = 0;
    for (; cid_len < bytes; cid_len++) {
        if (conn->sbuf[cid_len] == '\r' || conn->sbuf[cid_len] == '\n') {
            conn->sbuf[cid_len] = '\0';
            break;
        }
    }
    if (cid_len == bytes) ssh_fatal(conn); // no \r found

    // save the client id for later use
    conn->client_id = malloc(cid_len + 1);
    memcpy(conn->client_id, conn->sbuf, cid_len + 1);

    // and send ours
    int id_len = strlen(SERVER_ID);
    memcpy(conn->sbuf, SERVER_ID, id_len);
    conn->sbuf[id_len]     = '\r';
    conn->sbuf[id_len + 1] = '\n';
    if (send(conn->fd, conn->sbuf, id_len + 2, 0) == -1)
        ssh_fatal(conn);
}

// RFC 4253 / 7.
void algo_negotiation(connection *conn) {
    iter_t client_payload, server_payload;

    { // c2s
        iter_t packet = read_packet(conn);

        if (pop_byte(&packet) != SSH_MSG_KEXINIT) ssh_fatal(conn);
        packet.pos += 16; // skip over the client cookie

        // we only support one algorithm for everything, so we just have to check
        // if the client supports those too
        if (!namelist_has(pop_string(&packet),
                "diffie-hellman-group14-sha256") || // key exchange
            !namelist_has(pop_string(&packet),
                "ssh-rsa")                       || // server host key
            !namelist_has(pop_string(&packet),
                "aes256-ctr")                    || // c2s encryption
            !namelist_has(pop_string(&packet),
                "aes256-ctr")                    || // s2c encryption
            !namelist_has(pop_string(&packet),
                "hmac-sha2-256")                 || // c2s MAC
            !namelist_has(pop_string(&packet),
                "hmac-sha2-256")                 || // s2c MAC
            !namelist_has(pop_string(&packet),
                "none")                          || // c2s compression
            !namelist_has(pop_string(&packet),
                "none")                          ){ // s2c compression

            ssh_fatal(conn);
        }

        // then the client includes the languages that it supports
        // we don't give a fuck about those
        pop_string(&packet);
        pop_string(&packet);
        
        bool kex_follows = pop_byte(&packet);
        if (kex_follows) {
            puts("KEX follows and we don't support that yet"); // TODO like actually implement this or people will not be able to connect and you will spend ages figuring out why
            // the ruby implementation crashed, but YOLO let's just see what happens
        }
        pop_uint32(&packet); // "reserved"

        client_payload = iterator_copy(&packet);
    }
    { // s2c
        iter_t packet = start_packet(conn);

        push_byte(&packet, SSH_MSG_KEXINIT);
        packet.pos += 16; // TODO insert a random cookie

        // our algorithm preferences
        push_cstring(&packet, "diffie-hellman-group14-sha256"); // key exchange
        push_cstring(&packet, "ssh-rsa");                       // server host key
        push_cstring(&packet, "aes256-ctr");                    // c2s encryption
        push_cstring(&packet, "aes256-ctr");                    // s2c ^
        push_cstring(&packet, "hmac-sha2-256");                 // c2s mac
        push_cstring(&packet, "hmac-sha2-256");                 // s2c ^
        push_cstring(&packet, "none");                          // c2s compression
        push_cstring(&packet, "none");                          // s2c ^
        push_cstring(&packet, "");                              // c2s languages
        push_cstring(&packet, "");                              // s2c ^

        push_byte   (&packet, 0); // no kex packet follows
        push_uint32 (&packet, 0); // reserved

        packet.max = packet.pos;
        server_payload = iterator_copy(&packet);
        send_packet(conn, packet);
    }
}

// RFC 4253 / 8.
// diffie-hellman-group14-sha256
void key_exchange(connection *conn) {
    bignum *E = bignum_new(33); // must be as big as the DH prime

    { // 1. the client sends us E
        iter_t packet = read_packet(conn);
        if (pop_byte(&packet) != SSH_MSG_KEXDX_INIT) ssh_fatal(conn);
        pop_bignum(&packet, E);
    }
    { // 2. we send the signature
        iter_t packet = start_packet(conn);

        push_byte(&packet, SSH_MSG_KEXDX_REPLY);
        push_string(&packet, HOST_KEY.base, HOST_KEY.max); // TODO push_iter

    }

    free(E);
}
