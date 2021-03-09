#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t launch_shell() {
	pid_t child_pid = fork();

	if (!child_pid) {
		char* const shell[] = {"/bin/sh", NULL};
		int r = execve(shell[0], shell, NULL);

		if (r < 0) r = -r;
		while (r-- > 0) puts("+");

		puts("couldn't launch the shell");
		// todo: crash or something
	}

	return child_pid;
}

int main() {
	pid_t shell = launch_shell();

	int status;
	while (1) {
		pid_t reaped = wait(&status);
		if (reaped == shell) {
			puts("[init]\tshell died, relaunching");
			shell = launch_shell();
		}
	}
}
