#include <crypto/sha256.h>
#include <stdbool.h>

const char *s = "hello world";

bool test_sha256() {
    sha256_ctx ctx;
    char digest[32];

    sha256_init(&ctx);
    sha256_append(&ctx, s, 11);
    sha256_final(&ctx);
    sha256_digest(&ctx, &digest);
    hexdump(&digest, 32); // TODO add a normal check

    return true;
}
