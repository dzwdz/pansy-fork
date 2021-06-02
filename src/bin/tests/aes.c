#include <assert.h>
#include <crypto/aes.h>
#include <stdbool.h>
#include <string.h>

const char *key = "AES-256 needs a 32 byte long key";

void test_aes() {
    AES_ctx ctx;
    AES_init(&ctx, key, 256);

    uint8_t buf[16], expected[16];

    memcpy(buf,      "We have 16 chars", 16);
    memcpy(expected, "\x06u \xCDN\xE9h\x80S@\xF9\xF5}\xBF\x8D\xFB", 16);

    AES_ECB_encrypt(&ctx, buf);
    assert(memcmp(buf, expected, 16) == 0);
}
