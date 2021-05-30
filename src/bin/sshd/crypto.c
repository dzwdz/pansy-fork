#include "crypto.h"
#include "conn.h"
#include "misc.h"
#include <assert.h>
#include <bignum.h>
#include <crypto/sha1.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

bignum HOST_N; // modulus
bignum HOST_E; // public exponent
bignum HOST_D; // private exponent
iter_t HOST_KEY;

bignum DH14P;
bignum ONE; // TODO unnecessary
bignum TWO;

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
    HOST_N = BN_new(len);
    HOST_E = BN_new(1);
    HOST_D = BN_new(len);

    BN_fromhex(HOST_N, buf);
    BN_fromhex(HOST_E, p2);
    BN_fromhex(HOST_D, p3);

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

void diffie_hellman_group14(const bignum cl_pub, bignum our_pub,
                            bignum shared_secret) {
    bignum our_secret = BN_new(256 / 8);
    BN_random(ONE, DH14P, our_secret);

    BN_modexp_timingsafe(our_pub, TWO, our_secret, DH14P);
    BN_modexp_timingsafe(shared_secret, cl_pub, our_secret, DH14P);

    BN_free(our_secret);
}

// RFC 3447 / 9.2.
// assumes SHA1 as the function used
void pkcs1(iter_t msg, uint64_t length, void *buf) {
    uint64_t min_len =
        1   +  1    +  8       +  1    +  15              + 20;
    // 0x00 || 0x01 || PS (8+) || 0x00 || DigestInfo (15) || SHA1 (20)
    assert(length >= min_len);

    char *padded = buf;

    padded[0] = 0x00;
    padded[1] = 0x01;
    int i = 2;
    for (; i < length - 36; i++) {
        padded[i] = 0xFF;
    }

    // RFC 3447 / page 43
    memcpy(&padded[i], "\x00"
                       "\x30\x21\x30\x09\x06"
                       "\x05\x2b\x0e\x03\x02"
                       "\x1a\x05\x00\x04\x14", 16);
    i += 16;

    sha1_ctx ctx;
    sha1_init(&ctx);
    sha1_append(&ctx, msg.base, msg.max);
    sha1_final(&ctx);
    sha1_digest(&ctx, &padded[i]);
}

// RFC 3447 / 8.2.1.
// returns an iterator pointing to a freshly allocated buffer, remember to free it
// TODO it could just statically allocate a buffer in prepare_identities
iter_t RSA_sign(iter_t blob) {
    int length = 256; // TODO
    int overhead = 4 + 7 + 4 + 1; // 00 00 00 07  s  h  a  -  r  s  a  00 00 01 00
                                  // + a padding byte for the sig
    char *buf = malloc(length + overhead);
    bignum m = BN_new(256 / sizeof(uint64_t));
    bignum s = BN_new(256 / sizeof(uint64_t));

    pkcs1(blob, length, buf);

    // TODO BN_frombytes
    for (uint32_t i = 0; i < length; i++) {
        int reverse = length - i - 1;
        m.digits[reverse / 8]
            |= ((uint64_t)(buf[i]  & 0xff) << 8 * (reverse % 8));
    }


    BN_modexp_timingsafe(s, m, HOST_D, HOST_N);

    iter_t sig = {
        .base = (void *)buf,
        .max  = length + overhead,
        .pos  = 0
    };

    push_string(&sig, "ssh-rsa", 7);
    push_bignum(&sig, s);
    sig.max = sig.pos;

    free(m.digits);
    free(s.digits);

    return sig;
}

void init_crypto() {
    prepare_identities();

    // RFC 3526 / 3.
    DH14P = BN_new(256 / 8);
    BN_fromhex(DH14P, "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF");

    ONE = BN_new(1);
    BN_fromhex(ONE, "1");

    TWO = BN_new(1);
    BN_fromhex(TWO, "2");
}
