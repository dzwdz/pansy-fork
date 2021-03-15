#include <stdarg.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int64_t _syscall(int64_t number,
				 int64_t p1,
				 int64_t p2,
				 int64_t p3,
				 int64_t p4,
				 int64_t p5,
				 int64_t p6);

long syscall(long n, ...) {
	va_list ap;
	int64_t a, b, c, d, e, f;

	va_start(ap, n);
	a = va_arg(ap, int64_t);
	b = va_arg(ap, int64_t);
	c = va_arg(ap, int64_t);
	d = va_arg(ap, int64_t);
	e = va_arg(ap, int64_t);
	f = va_arg(ap, int64_t);
	va_end(ap);

	return _syscall(n, a, b, c, d, e, f);
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
	return syscall(SYS_read, fildes, buf, nbyte);
}

ssize_t write(int fildes, const void *buf, size_t nbyte) {
	return syscall(SYS_write, fildes, buf, nbyte);
}

int execve(const char *path, char *const argv[], char *const envp[]) {
	return syscall(SYS_execve, path, argv, envp);
}

pid_t wait(int *wstatus) {
	return waitpid((pid_t)-1, wstatus, 0);
}

pid_t waitpid(pid_t pid, int *wstatus, int options) {
	return syscall(SYS_wait4, pid, wstatus, options, NULL);
}

pid_t fork(void) {
	return syscall(SYS_fork);
}


char *getcwd(char *buf, size_t size) {
	return (char*) syscall(SYS_getcwd, buf, size);
}

void *mmap(void *addr, size_t len, int prot, int flags,
           int fildes, off_t off) {
	return (void*) syscall(SYS_mmap, addr, len, prot, flags, fildes, off);
}

void *sbrk(intptr_t increment) {
	void* brk = (void*) syscall(SYS_brk, 0);
	return (void*) syscall(SYS_brk, brk + increment);
}

int chdir(const char *path) {
	return syscall(SYS_chdir, path);
}

void exit(int status) {
	syscall(SYS_exit, status);
	__builtin_unreachable();
}

int close(int fd) {
	return syscall(SYS_close, fd);
}

int stat(const char *path, struct stat *statbuf) {
	return syscall(SYS_stat, path, statbuf);
}

int dup2(int oldfd, int newfd) {
	return syscall(SYS_dup2, oldfd, newfd);
}


int socket(int domain, int type, int protocol) {
	return syscall(SYS_socket, domain, type, protocol);
}

int bind(int socket, const struct sockaddr *address, socklen_t addr_len) {
	return syscall(SYS_bind, socket, address, addr_len);
}

int listen(int socket, int backlog) {
	return syscall(SYS_listen, socket, backlog);
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	return syscall(SYS_accept, sockfd, addr, addrlen);
}

ssize_t sendto(int socket, const void *buffer, size_t len, int flags,
             const struct sockaddr *dest_addr, socklen_t addrlen) {
	return syscall(SYS_sendto, socket, buffer, len, flags, dest_addr, addrlen);
}

ssize_t send(int socket, const void *buffer, size_t len, int flags) {
	return sendto(socket, buffer, len, flags, 0, 0);
}


int ioctl(int fd, unsigned long req, ...) {
	va_list ap;
	va_start(ap, req);
	void *arg = va_arg(ap, void *);
	va_end(ap);

	// musl does some more stuff here, i might be missing something?
	return syscall(SYS_ioctl, fd, req, arg);
}

int mknod(const char *pathname, mode_t mode, dev_t dev) {
	return syscall(SYS_mknod, pathname, mode, dev);
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
	return syscall(SYS_nanosleep, req, rem);
}
