#include <crypto/sha1.h>
#include <crypto/sha256.h>
#include <stdbool.h>

const char *s = "hello world";

bool test_sha256() {
    char digest[32];

    {
        sha256_ctx ctx;
        sha256_init(&ctx);
        sha256_append(&ctx, s, 11);
        sha256_final(&ctx);
        sha256_digest(&ctx, &digest);
        hexdump(&digest, 32); // TODO add a normal check
    }

    {
        sha1_ctx ctx;
        sha1_init(&ctx);
        sha1_append(&ctx, s, 11);
        sha1_final(&ctx);
        sha1_digest(&ctx, &digest);
        hexdump(&digest, 20); // TODO add a normal check
    }

    return true;
}
