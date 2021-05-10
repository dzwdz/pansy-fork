#pragma once

#include <stddef.h>
#include <sys/types.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef long int intptr_t; /* i hope this works */

ssize_t read(int fd, void *buf, size_t nbytes);
ssize_t write(int fd, const void *buf, size_t n);
int close(int fd);
off_t lseek(int fd, off_t offset, int whence);
pid_t fork(void);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int execve(const char *pathname, char *const argv[], char *const envp[]);
int unlink(const char *pathname);
int brk(void *addr);
void *sbrk(intptr_t increment);
char *getcwd(char *buf, size_t size);
uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int setreuid(uid_t ruid, uid_t euid);
int setregid(gid_t rgid, gid_t egid);
int rmdir(const char *pathname);
int chdir(const char *path);
// might be in the wrong header
int mknod(const char *pathname, mode_t mode, dev_t dev);

int getentropy(void *buffer, size_t length);
