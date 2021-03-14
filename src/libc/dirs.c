#include "dirs.h"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

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
