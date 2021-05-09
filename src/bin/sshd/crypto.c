#include "crypto.h"
#include "conn.h"
#include "misc.h"
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

iter_t HOST_KEY;

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

    close(fd);
    free(buf);


    HOST_KEY.base = malloc(32 + len * 8); // also should be enough i guess
    HOST_KEY.max = 32 + len * 8;
    HOST_KEY.pos = 0;
    
    push_cstring(&HOST_KEY, "ssh-rsa");
    push_bignum(&HOST_KEY, HOST_E);
    push_bignum(&HOST_KEY, HOST_N);

    HOST_KEY.max = HOST_KEY.pos;
    hexdump(HOST_KEY.base, HOST_KEY.max);
}
