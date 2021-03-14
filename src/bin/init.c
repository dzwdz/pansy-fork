#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t launch_login() {
	pid_t child_pid = fork();

	if (!child_pid) {
		char* const login[] = {"/bin/login", NULL};
		int r = execve(login[0], login, NULL);

		if (r < 0) r = -r;
		while (r-- > 0) puts("+");

		puts("couldn't launch the login");
		// todo: crash or something
	}

	return child_pid;
}

int main() {
	pid_t login = launch_login();

	int status;
	while (1) {
		pid_t reaped = wait(&status);
		if (reaped == login) {
			login = launch_login();
		}
	}
}
