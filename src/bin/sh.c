#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

// todo tidy this up
// bad code ahead
// also i'm not freeing any memory :) TODO a malloc

char** env;

void readline(char* buf, size_t buf_size) {
	size_t i = 0;
	for (; i < buf_size; i++) {
		read(STDIN_FILENO, &buf[i], 1);
		if (buf[i] == '\n') break;
	}
	buf[i] = '\0';
}

char** split_args(const char* str) {
	char** parts_start = sbrk(64 * sizeof(char*));
	char** parts       = parts_start;

	char* buf_start    = sbrk(strlen(str) + 64);
	char* buf          = buf_start;

	bool quoted        = false;

	while (*str != '\0') {
		switch (*str) {
			// WHITESPACE
			case ' ':
			case '\n':
			case '\t':
				// if we're in a quote, just add the whitespace to the buffer
				if (quoted) {
					*buf++ = *str;
					break;
				}

				// ignore repeating whitespace
				if (buf == buf_start) break;

				*buf++ = '\0';
				*parts++ = buf_start;
				buf_start = buf;
				break;

			// QUOTES
			case '"':
				quoted ^= true;
				if (!quoted) {
					*buf++ = '\0';
					*parts++ = buf_start;
					buf_start = buf;
				}
				break;

			// ESCAPED CHARS
			case '\\': {
				char c = *++str;
				switch (c) {
					case 't': c = '\t'; break;
					case 'r': c = '\r'; break;
					case 'n': c = '\n'; break;
					case '0': c = '\0'; break;
				}
				*buf++ = c;
				break;
			}

			default:
				*buf++ = *str;
		}
		str++;
	}

	if (buf != buf_start)  {
		*buf++ = '\0';
		*parts++ = buf_start;
	}

	*parts = NULL;
	return parts_start;
}

int run(char** args) {
	// some builtins
	if (!strcmp(args[0], "exit"))
		exit(0);

	if (!strcmp(args[0], "cd")) {
		chdir(args[1]);
		return 0;
	}

	// the command isn't a builtin, try running an executable
	if (!fork()) {
		execve(args[0], args, env);
		puts("pansh: execve() error");
	}

	int status;
	wait(&status);
	return status;
}

int main(int argc __attribute__((unused)),
	     char** argv __attribute__((unused)),
	     char** envp) {

	env = envp;

	size_t buf_s = 128;
	char *buf = sbrk(buf_s);

	while (1) {
		getcwd(buf, buf_s);
		puts(buf);

		write(1, "; ", 2);

		readline(buf, buf_s);
		if (buf[0] == '\0')
			continue;

		char** args = split_args(buf);
		run(args);
	}

	return 0;
}
