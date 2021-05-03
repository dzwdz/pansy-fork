/*
 * this is just a very verbose rewrite of my simple_ssh server
 * https://github.com/dzwdz/simple_ssh/blob/main/server.rb
 *
 * if you want to understand what's happening here you should take a look
 * at it first
 */

#include "conn.h"
#include "proto.h"
#include <arpa/inet.h>
#include <macros.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <tty.h>
#include <unistd.h>

const char *SERVER_ID = "SSH-2.0-pansy";

void handle_client(int fd) {
    connection conn;
    conn.fd = fd;
    conn.sbuf = malloc(SBUF_SIZE);

    id_exchange(&conn);
    printf("new connection from %s\n", conn.client_id);

    algo_negotiation(&conn);

    free(conn.sbuf);
    // fd gets closed by server_loop
    // not that it matters, as it exits right after
}

// listens on the given port
// when a client connects it forks and calls handle_client()
void server_loop(unsigned short port) {
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) DIE("couldn't create the socket", 1);

    struct sockaddr_in my_addr = {0};
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (void*)&my_addr, sizeof my_addr) == -1)
        DIE("couldn't bind socket", 1);
    if (listen(sockfd, 10) == -1)
        DIE("couldn't listen to the socket", 1);

    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof client_addr;
    
    printf("listening on :%d\n", port);
    for (;;) {
        int client_fd = accept(sockfd, (void*)&client_addr, &addr_size);

        if (fork() == 0) {
            handle_client(client_fd);
            close(client_fd);
            exit(0);
        } else {
            close(client_fd);
            exit(0); // TODO this is convinent for debugging
        }
    }
}

int main() {
    server_loop(2020);
}
