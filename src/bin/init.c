#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <syscall.h>
#include <unistd.h>

void load_module(const char *path) {
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

pid_t launch(char *path, bool daemonize) {
	pid_t pid = fork();
	if (pid != 0) return pid;

	// if you'll be copying this to somewhere else - change sid
	if (daemonize) {
		// redirect std* to /dev/null
		int fd = open("/dev/null", O_RDWR);
		if (fd < 0) exit(1); // todo this really needs proper error handling
		if (dup2(fd, 0) < 0
		 || dup2(fd, 1) < 0
		 || dup2(fd, 2) < 0) exit(1);

		if (fd > 2) close(fd);
	}

	char* const args[] = {path, NULL};
	int r = execve(path, args, NULL);

	// couldn't launch, todo print a proper log message
	if (r < 0) r = -r;
	while (r-- > 0) puts("+");
	printf("oh no, %s\n", path);
	while (1) {}

	exit(1);
}

int main() {
	puts("[init]\tcreating devices");
	mknod("/dev/null", S_IFCHR, 0x103);

	puts("[init]\tloading kernel modules");
	load_module("/lib/modules/e1000.ko");

	puts("[init]\tsummoning daemons");
	launch("/bin/ethup", true);
	launch("/bin/httpd", true);

	puts("[init]\tlaunching login");
	pid_t login = launch("/bin/login", false);

	int status;
	while (1) {
		pid_t reaped = wait(&status);
		if (reaped == login) {
			login = launch("/bin/login", false);
		}
	}
}
