#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
	puts("hi from init!");

	if (!fork()) {
		puts("about to launch the shell");

		char* const shell[] = {"/sh", NULL};
		int r = execve(shell[0], shell, NULL);

		if (r < 0) r = -r;
		while (r-- > 0) puts("+");

		puts("execve() failed");
	}

	int status;
	while (1)
		wait(&status);
}
