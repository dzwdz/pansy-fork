#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

void *malloc(size_t size);
void free(void *ptr);

int atoi(const char *str);

void exit(int status);

#endif /* STDLIB_H */
