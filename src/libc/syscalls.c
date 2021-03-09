#include <stdarg.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/syscall.h>
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
