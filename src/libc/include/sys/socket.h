#pragma once

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#define SOCK_STREAM 1
#define SOCK_DGRAM  2

#define PF_INET 2
#define AF_INET PF_INET

#define INADDR_ANY 0

typedef uint16_t sa_family_t;
typedef uint16_t in_port_t;

struct sockaddr {
        sa_family_t sa_family;
        char sa_data[14];
};

struct in_addr { uint32_t s_addr; };

struct sockaddr_in {
        sa_family_t sin_family;
        in_port_t sin_port;
        struct in_addr sin_addr;
        uint8_t sin_zero[8];
};

struct sockaddr_storage {
        sa_family_t ss_family;
        char __ss_padding[128-sizeof(long)-sizeof(sa_family_t)];
        unsigned long __ss_align;
};

int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict addrlen);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
