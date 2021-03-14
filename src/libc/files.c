#include <fcntl.h>
#include <sys/syscall.h>
#include "syscalls.h"

// TODO this should do more stuff
int open(const char *path, int flags, ...) {
	return syscall(SYS_open, path, flags, 0);
}
