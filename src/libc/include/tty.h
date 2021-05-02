#pragma once
#include <stddef.h>

void readline(char* buf, size_t buf_size);
// prints an OpenSSL style hexdump
void hexdump(const void *buf, size_t len);
