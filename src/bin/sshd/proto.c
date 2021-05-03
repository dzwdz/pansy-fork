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
    iter_t packet = read_packet(conn);

    if (pop_byte(&packet) != SSH_MSG_KEXINIT) ssh_fatal(conn);
    packet.pos += 16; // skip over the client cookie

    iter_t kex_methods = pop_string(&packet);
    hexdump(kex_methods.base, kex_methods.max);
}
