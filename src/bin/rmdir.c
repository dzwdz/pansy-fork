#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

// TOOD: print errors to stderr
void rm_path(char **paths) {
    struct stat s;
    char *d;

    while ((d = *paths++) != NULL) {
        int status = -lstat(d, &s);
        if (status == 2) {
            printf("rmdir: Cannot remove %s, no such file or directory.", d);
        } else if (!S_ISDIR(s.st_mode)) {
            printf("rmdir: %s is not a directory.\n", d);
        } else if (status) {
            printf("rmdir: stat error: %d\n", status);
        } else {
            int n = -rmdir(d);
            if (n) {
                printf("rmdir: error removing %s: %d\n", d, n);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: rmdir DIRECTORY...");
        return 0;
    }
    argv++;
    rm_path(argv);
}
