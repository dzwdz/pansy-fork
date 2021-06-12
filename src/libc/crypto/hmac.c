#include <assert.h>
#include <crypto/hmac.h>
#include <crypto/sha256.h>
#include <string.h>

void HMAC_SHA256(const void *key, size_t keylen,
                 const void *msg, size_t msglen,
                 void *out /*32b*/)
{
    HMAC_SHA256_prefix(key, keylen, NULL, 0, msg, msglen, out);
}

void HMAC_SHA256_prefix
                (const void *key, size_t keylen,
                 const void *pfx, size_t pfxlen,
                 const void *msg, size_t msglen,
                 void *out /*32b*/)
{
    sha256_ctx sha;

    char padded[64];
    assert(keylen <= 64);

    memcpy(padded, key, keylen);
    memset(&padded[keylen], 0, 64 - keylen);

    for (int i = 0; i < 64; i++) padded[i] ^= 0x36; // ipad

    sha256_init(&sha);
    sha256_append(&sha, padded, 64);
    if (pfxlen != 0) {
        sha256_append(&sha, pfx, pfxlen);
    }
    sha256_append(&sha, msg, msglen);
    sha256_final(&sha);
    sha256_digest(&sha, out);

    for (int i = 0; i < 64; i++) padded[i] ^= 0x36 ^ 0x5C; // opad

    sha256_init(&sha);
    sha256_append(&sha, padded, 64);
    sha256_append(&sha, out, 32);
    sha256_final(&sha);
    sha256_digest(&sha, out);
}
