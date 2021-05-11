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

bignum *DH14P;
bignum *ONE;
bignum *TWO;

static void prepare_identities() {
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


    // RFC 4253 / 6.6.
    HOST_KEY.base = malloc(32 + len * 8); // also should be enough i guess
    HOST_KEY.max = 32 + len * 8;
    HOST_KEY.pos = 0;
    
    push_cstring(&HOST_KEY, "ssh-rsa");
    push_bignum(&HOST_KEY, HOST_E);
    push_bignum(&HOST_KEY, HOST_N);

    HOST_KEY.max = HOST_KEY.pos;
}

void diffie_hellman_group14(const bignum *cl_pub, bignum *our_pub,
                            bignum *shared_secret) {
    bignum *our_secret = bignum_new(256 / 8);
    bignum_random(ONE, DH14P, our_secret);

    bignum_modexp_timingsafe(our_pub, TWO, our_secret, DH14P);
    bignum_modexp_timingsafe(shared_secret, cl_pub, our_secret, DH14P);

    free(our_secret);
}

void init_crypto() {
    prepare_identities();

    // RFC 3526 / 3.
    DH14P = bignum_new(256 / 8);
    bignum_fromhex(DH14P, "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF");

    ONE = bignum_new(1);
    bignum_fromhex(ONE, "1");

    TWO = bignum_new(1);
    bignum_fromhex(TWO, "2");
}
