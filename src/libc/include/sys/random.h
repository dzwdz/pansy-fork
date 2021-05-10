#pragma once
#include <sys/types.h>

ssize_t getrandom(void *buf, size_t buflen, unsigned int flags);
