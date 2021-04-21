#include <stdbool.h>
#pragma once

// copied from the man getdents(2)
struct dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen;  /* Length of this linux_dirent */
    char           d_name[];  /* Filename (null-terminated) */
                     /* length is actually (d_reclen - 2 -
                        offsetof(struct linux_dirent, d_name)) */
    /*
    char           pad;       // Zero padding byte
    char           d_type;    // File type (only since Linux
                             // 2.6.4); offset is (d_reclen - 1)
    */
};

enum {
    DT_UNKNOWN = 0,
    DT_FIFO = 1,
    DT_CHR = 2,
    DT_DIR = 4,
    DT_BLK = 6,
    DT_REG = 8,
    DT_LNK = 10,
    DT_SOCK = 12,
    DT_WHT = 14
};

typedef struct {
    int fd;
    int buf_pos;
    int buf_end;
    char buf[2048];
} DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR* dir);
int closedir(DIR* dir);

bool is_path_safe(const char *c);
