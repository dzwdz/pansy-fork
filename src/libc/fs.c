#include "fs.h"
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

/***  SYSCALLS  ***/
char *getcwd(char *buf, size_t size) {
	return (char*) syscall(SYS_getcwd, buf, size);
}

int chdir(const char *path) {
	return syscall(SYS_chdir, path);
}

int mount(const char *source, const char *target, const char *fstype,
          unsigned long flags, const void *data) {
	return syscall(SYS_mount, source, target, fstype, flags, data);
}

int mkdir(const char *pathname, mode_t mode) {
	return syscall(SYS_mkdir, pathname, mode);
}

// TODO this should do more stuff
int open(const char *path, int flags, ...) {
	return syscall(SYS_open, path, flags, 0);
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

int stat(const char *path, struct stat *statbuf) {
	return syscall(SYS_stat, path, statbuf);
}

int fstat(int fd, struct stat *statbuf) {
	return syscall(SYS_fstat, fd, statbuf);
}

int chown(const char *pathname, uid_t owner, gid_t group) {
	return syscall(SYS_chown, pathname, owner, group);
}


/***  DIRECTORY STUFF  ***/
DIR *opendir(const char *name) {
	int fd = open(name, O_RDONLY|O_DIRECTORY|O_CLOEXEC);
	if (fd < 0)
		return 0;

	DIR *dir = malloc(sizeof(DIR));
	if (!dir) {
		close(fd);
		return 0;
	}

	dir->fd = fd;
	dir->buf_pos = 0;
	dir->buf_end = 0;
	return dir;
}

struct dirent *readdir(DIR* dir) {
	if (dir->buf_pos >= dir->buf_end) {
		dir->buf_end = \
			syscall(SYS_getdents, dir->fd, dir->buf, sizeof dir->buf);
		if (dir->buf_end <= 0) {
			// todo check errno
			return 0;
		}
		dir->buf_pos = 0;
	}

	struct dirent *de = (void*)&dir->buf[dir->buf_pos];
	dir->buf_pos += de->d_reclen;
	return de;
}

int closedir(DIR* dir) {
	close(dir->fd);
	free(dir);
	return 0;
}
