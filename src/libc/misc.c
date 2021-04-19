#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

void __libc_start_main(int argc, char** argv,
		int (*main)(int, char**, char**)) {
	char **envp = argv+argc+1;

	exit(main(argc, argv, envp));
}

int putchar(int c) {
	return write(1, &c, 1);
}

int puts(const char *s) {
	int b = write(1, s, strlen(s));
	if (b < 0) return b;
	return b + write(1, "\n", 1);
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
			case 'c': {
				char c = va_arg(argp, int);
				write(1, &c, 1);
				break;}
			case 'x': {
				unsigned int num = va_arg(argp, unsigned int);

				int i = 0;
				while (num >> i && i < (sizeof(int) * 8)) i += 4;
				if (i == 0) i = 4;
				
				write(1, "0x", 2);
				while (i > 0) {
					i -= 4;
					char c = '0' + ((num >> i) & 0xf);
					if (c > '9') c += 'A' - '9' - 1;
					write(1, &c, 1);
				}

				break;}
			case 'd': {
				char c;
				int n = va_arg(argp, int);

				// special case for when the number is zero
				if (n == 0) {
					c = '0';
					write(1, &c, 1);
					break;
				}
				else if (n < 0) {
					c = '-';
					write(1, &c, 1);
					n = -n;
				}

				// write to string
				char to_print[10] = {0};
				int i = 0;
				while (n != 0) {
					to_print[i++] = (n % 10) + '0';
					n /= 10;
				}

				// that previous string is reversed, so we print it backwards
				for (int k = strlen(to_print); k > 0; k--) {
					c = to_print[k - 1];
					write(1, &c, 1);
				}

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

bool is_number(const char *str) {
	if (!((*str > '0' && *str < '9') || *str == '-'))
		return false;
	if ((*str == '-') && (strlen(str) == 1))
		return false;
	for (size_t i = 1; i < strlen(str); i++) {
		if (!(str[i] > '0' && str[i] < '9'))
			return false;
	}
	return true;
}

int atoi(const char *str) {
	if (!is_number(str))
		return 0;
	int n = 0;
	int sign = 1;
	int i = 0;
	if (str[i] == '-') {
		sign = -1;
		i++;
	}
	for (; str[i] != '\0'; i++) {
		n = n * 10 + str[i] - '0';
	}
	return n * sign;
}
