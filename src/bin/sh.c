#include <stdio.h>
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

	while (*str != '\0') {
		switch (*str) {
			case ' ':
			case '\n':
			case '\t':
				*buf++ = '\0';
				*parts++ = buf_start;
				buf_start = buf;
				break;

			// todo support escaping and quotes
			// also, repeating whitespace
			default:
				*buf++ = *str;
		}
		str++;
	}

	*buf++ = '\0';
	*parts++ = buf_start;
	*parts = NULL;

	return parts_start;
}

int run(char** args) {
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
		char** args = split_args(buf);

		run(args);
	}

	return 0;
}
