#include <stdio.h>
#include <sys/wait.h>
#include <syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

void load_module(const char* path) {
	int fd = open(path, 0);
	if (fd == -1) {
		printf("couldn't find kernel module %s\n", path);
		exit(1);
	}

	if (syscall(SYS_finit_module, fd, "", 0) == -1) {
		printf("couldn't load kernel module %s\n", path);
		exit(1);
	}

	close(fd);
	//printf("succesfully loaded %s\n", path);
}

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
	load_module("/lib/modules/e1000.ko");

	pid_t login = launch_login();

	int status;
	while (1) {
		pid_t reaped = wait(&status);
		if (reaped == login) {
			login = launch_login();
		}
	}
}
