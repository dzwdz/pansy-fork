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

int strcmp(const char *s1, const char *s2) {
	do {
		if (*s1 != *s2++)
			return s1 - s2;
	} while (*s1++);

	return 0;
}
