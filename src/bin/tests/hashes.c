#include <assert.h>
#include <crypto/sha1.h>
#include <crypto/sha256.h>
#include <stdbool.h>
#include <string.h>

const char *s = "hello world";

void test_sha() {
    char digest[32];

    { // sha256("hello world") == B94D27...
        sha256_ctx ctx;
        sha256_init(&ctx);
        sha256_append(&ctx, s, 11);
        sha256_final(&ctx);
        sha256_digest(&ctx, &digest);
        assert(memcmp(digest, "\xB9\x4D\x27\xB9\x93\x4D\x3E\x08\xA5\x2E\x52\xD7\xDA\x7D\xAB\xFA\xC4\x84\xEF\xE3\x7A\x53\x80\xEE\x90\x88\xF7\xAC\xE2\xEF\xCD\xE9", 32) == 0);
    }

    { // sha1("hello world") == 2AAE6C...
        sha1_ctx ctx;
        sha1_init(&ctx);
        sha1_append(&ctx, s, 11);
        sha1_final(&ctx);
        sha1_digest(&ctx, &digest);
        assert(memcmp(digest, "\x2A\xAE\x6C\x35\xC9\x4F\xCF\xB4\x15\xDB\xE9\x5F\x40\x8B\x9C\xE9\x1E\xE8\x46\xED", 20) == 0);
    }
}
