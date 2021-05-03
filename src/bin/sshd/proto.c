/*
 * here are the functions that handle the "higher-level" parts of the SSH
 * protocol
 */

#include "conn.h"
#include "magics.h"
#include "misc.h"
#include "proto.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

#include <tty.h>

void id_exchange(connection *conn) {
    int bytes = recv(conn->fd, conn->sbuf, 2048, 0);
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

    // and send ours (this is ugly but it works sooo..)
    dprintf(conn->fd, "%s\r\n", SERVER_ID);
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

    hexdump(client_payload.base, client_payload.max);
}
