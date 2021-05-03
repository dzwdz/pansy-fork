#pragma once

typedef struct {
    int fd;
    char *sbuf; // 2048 bytes in size, no guarantees about what's in it
                // i'm just using it to avoid constant mallocs

    // id_exchange
    char *client_id;
} connection;

void ssh_fatal(connection *conn);
