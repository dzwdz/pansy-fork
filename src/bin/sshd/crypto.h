#pragma once
#include <bignum.h>
#include "misc.h"

extern iter_t HOST_KEY;

void init_crypto();
void diffie_hellman_group14(const bignum cl_pub, bignum our_pub,
                            bignum shared_secret);
