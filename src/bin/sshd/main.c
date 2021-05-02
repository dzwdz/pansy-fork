/*
 * this is just a very verbose rewrite of my simple_ssh server
 * https://github.com/dzwdz/simple_ssh/blob/main/server.rb
 *
 * if you want to understand what's happening here you should take a look
 * at it first
 */

#include <arpa/inet.h>
#include <macros.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <tty.h>
#include <unistd.h>

const char *server_id = "SSH-2.0-pansy";

void handle_client(int fd) {
    char *buf = malloc(2048);

    { // ID exchange
        int bytes = recv(fd, buf, 1024, 0);
        if (bytes <= 0) return; // note: i don't care about freeing buf as
                                // this process will quit right after this return

        // TODO this is not spec compliant
        // we find the first \r - it *should* be right after the client id
        int cid_len = 0;
        for (; cid_len < bytes; cid_len++) {
            if (buf[cid_len] == '\r') {
                buf[cid_len] = '\0';
                break;
            }
        }
        if (cid_len == bytes) return; // no \r found

        printf("new connection from %s\n", buf);
    }
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
    if (listen(sockfd, 10) == -1) DIE("couldn't listen to the socket", 1);

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
        }
    }
}

int main() {
    server_loop(2020);
}
