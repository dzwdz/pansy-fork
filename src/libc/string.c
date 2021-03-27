#include <string.h>
#include <unistd.h>

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

char *strcpy(char *dest, const char *src) {
	char *og = dest;
	while ((*dest++ = *src++));
	return og;
}
