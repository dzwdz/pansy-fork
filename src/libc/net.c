#include <stdint.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <unistd.h>

/***  SYSCALLS  ***/
int socket(int domain, int type, int protocol) {
    return syscall(SYS_socket, domain, type, protocol);
}

int bind(int socket, const struct sockaddr *address, socklen_t addr_len) {
    return syscall(SYS_bind, socket, address, addr_len);
}

int listen(int socket, int backlog) {
    return syscall(SYS_listen, socket, backlog);
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    return syscall(SYS_accept, sockfd, addr, addrlen);
}

ssize_t sendto(int socket, const void *buffer, size_t len, int flags,
             const struct sockaddr *dest_addr, socklen_t addrlen) {
    return syscall(SYS_sendto, socket, buffer, len, flags, dest_addr, addrlen);
}

ssize_t send(int socket, const void *buffer, size_t len, int flags) {
    return sendto(socket, buffer, len, flags, 0, 0);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen) {
    return syscall(SYS_recvfrom, sockfd, buf, len, flags, src_addr, addrlen);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return recvfrom(sockfd, buf, len, flags, 0, 0);
}


/*** MISC ***/
uint16_t htons(uint16_t hosts) {
    return hosts << 8 | hosts >> 8;
}
