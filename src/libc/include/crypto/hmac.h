#pragma once
#include <stddef.h>

void HMAC_SHA256(const void *key, size_t keylen,
                 const void *msg, size_t msglen,
                 void *out /*32b*/);

void HMAC_SHA256_prefix
                (const void *key, size_t keylen,
                 const void *pfx, size_t pfxlen,
                 const void *msg, size_t msglen,
                 void *out /*32b*/);
