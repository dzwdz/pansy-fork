#include <stdarg.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

int64_t _syscall(int64_t number,
				 int64_t p1,
				 int64_t p2,
				 int64_t p3,
				 int64_t p4,
				 int64_t p5);

long syscall(long n, ...) {
	va_list ap;
	int64_t a,b,c,d,e;

	va_start(ap, n);
	a = va_arg(ap, int64_t);
	b = va_arg(ap, int64_t);
	c = va_arg(ap, int64_t);
	d = va_arg(ap, int64_t);
	e = va_arg(ap, int64_t);
	va_end(ap);

	return _syscall(n, a, b, c, d, e);
}

ssize_t write(int fildes, const void *buf, size_t nbyte) {
	return syscall(SYS_write, fildes, buf, nbyte);
}
