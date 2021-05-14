#pragma once

#include <stddef.h>

#define EOF -1

int putchar(int c);
int puts(const char *s);
int printf(const char *fmt, ...);
int dprintf(int fd, const char *fmt, ...);
int sprintf(char *out_str, const char *fmt, ...);
