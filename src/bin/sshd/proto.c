/*
 * here are the functions that handle the "higher-level" parts of the SSH
 * protocol
 *
 * and some helper ones too, such as ssh_fatal, which will be moved to conn->c
 * someday
 */

#include "conn.h"
#include "proto.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

void id_exchange(connection *conn) {
    int bytes = recv(conn->fd, conn->sbuf, 1024, 0);
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

void ssh_fatal(connection *conn) {
    puts("closing connection due to some fatal error");
    close(conn->fd);
    free(conn->sbuf);
    exit(1);
}
