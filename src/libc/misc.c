#include <stdarg.h>
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

char *strcpy(char *dest, const char *src) {
	char *og = dest;
	while ((*dest++ = *src++));
	return og;
}

int printf(const char *fmt, ...) {
	const char *sub = fmt;
	int total = 0;

	va_list argp;
	va_start(argp, fmt);

	while (1) {
		char c = *fmt++;
		switch (c) {
		case '%':
			write(1, sub, fmt - sub - 1);
			total += fmt - sub - 1;

			c = *fmt++;
			switch (c) {
			case 's': {
				const char *s = va_arg(argp, char*); 
				write(1, s, strlen(s));
				break;}
			}

			sub = fmt;
			break;

		case '\0':
			write(1, sub, fmt - sub);
			return total + (fmt - sub);
		}
	}

	va_end(argp);
}
