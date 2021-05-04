#include <crypto/sha256.h>
#include <stdbool.h>

const char *s = "this string is very long, it has exactly 64 characters. fuck yea. update, my needs have changed. now it has 128 chars. fuck yeah";

bool test_sha256() {
    sha256_ctx ctx;
    sha256_init(&ctx);
//  sha256_append(&ctx, s, 128);
    sha256_final(&ctx);
    hexdump(&ctx, 64);

    return true;
}
