#include <stdio.h>
#include <string.h>
#include <unistd.h>

int puts(const char *s) {
	int b = write(1, s, strlen(s));
	if (b < 0) return b;
	return b + write(1, "\n", 1);
}

size_t strlen(const char *s) {
	size_t c = 0;
	while (*s++) c++;
	return c;
}
