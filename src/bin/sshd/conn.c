#include "conn.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>

// reads a single packet into conn.sbuf
// RFC 4253 / 6.
iter_t read_packet(connection *conn) {
    ssize_t bytes = recv(conn->fd, conn->sbuf, 2048, 0);
    if (bytes < 5) ssh_fatal(conn);

    // the packet size is at the beginning of the buffer
    uint32_t packet_l = ntohl(*(uint32_t*)conn->sbuf);
    // and the padding size is right after it
    uint8_t padding_l = conn->sbuf[4];
    printf("%d + %d\n", packet_l, padding_l);

    if (bytes < packet_l + 4 /* + mac_l*/) {
        puts("fix me you dumbass"); // TODO
        ssh_fatal(conn);
    }

    iter_t iter;
    iter.base = conn->sbuf + 5;
    iter.max = packet_l - padding_l - 1; // TODO check for overflow
    iter.pos = 0;
    return iter;
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


void ssh_fatal(connection *conn) {
    puts("closing connection due to some fatal error");
    close(conn->fd);
    free(conn->sbuf);
    exit(1);
}

