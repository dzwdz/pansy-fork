#include "crypto.h"
#include <bignum.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

bignum *HOST_N; // modulus
bignum *HOST_E; // public exponent
bignum *HOST_D; // private exponent

void prepare_identities() {
    // assert HOST_N == null
    
    int fd = open("static/etc/sshd/id_rsa", O_RDONLY);
    if (fd == -1) {
        puts("couldn't open /etc/sshd/id_rsa, quitting");
        exit(1);
    }

    // TODO rewrite this - this is brittle as fuck
    char *buf = malloc(1200); // should be enough?
    read(fd, buf, 1200);

    char *p2, *p3, *p4;
    p2 = buf;
    while (*p2++ != '\n') {}
    p2[-1] = '\0';
    p3 = p2;
    while (*p3++ != '\n') {}
    p3[-1] = '\0';
    p4 = p3;
    while (*p4++ != '\n') {}
    p4[-1] = '\0';

    int len = strlen(buf) / 2 / 8 + 1; // slapping a 1 there for good measure
    HOST_N = bignum_new(len);
    HOST_E = bignum_new(1);
    HOST_D = bignum_new(len);

    bignum_fromhex(HOST_N, buf);
    bignum_fromhex(HOST_E, p2);
    bignum_fromhex(HOST_D, p3);

    bignum_print(HOST_N);
    bignum_print(HOST_E);
    bignum_print(HOST_D);

    close(fd);
}
