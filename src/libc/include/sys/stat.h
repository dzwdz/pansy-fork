#pragma once

#include <time.h> /* timespec */
#include <sys/types.h>

#define S_IFMT  0170000

#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFBLK 0060000
#define S_IFREG 0100000
#define S_IFIFO 0010000
#define S_IFLNK 0120000
#define S_IFSOCK 0140000

#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#define S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)
#define S_ISBLK(mode)  (((mode) & S_IFMT) == S_IFBLK)
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define S_ISLNK(mode)  (((mode) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)

struct stat {
        dev_t st_dev;
        ino_t st_ino;
        nlink_t st_nlink;

        mode_t st_mode;
        uid_t st_uid;
        gid_t st_gid;
        unsigned int    __pad0;
        dev_t st_rdev;
        off_t st_size;
        blksize_t st_blksize;
        blkcnt_t st_blocks;

        struct timespec st_atim;
        struct timespec st_mtim;
        struct timespec st_ctim;
        long __unused[3];
};

int stat(const char *restrict pathname, struct stat *restrict statbuf);
int fstat(int fd, struct stat *statbuf);
int lstat(const char *restrict path, struct stat *restrict buf);
int mkdir(const char *pathname, mode_t mode);
