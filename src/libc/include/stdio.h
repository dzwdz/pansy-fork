#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>

#define EOF -1

int putchar(int c);
int puts(const char *s);
int printf(const char *fmt, ...);

#endif /* STDIO_H */
