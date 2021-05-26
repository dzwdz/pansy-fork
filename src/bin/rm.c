#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

void rm_file(char **files) {
    struct stat s;
    char *f;

    while ((f = *files++) != NULL) {
        int status = -lstat(f, &s);
        if (status == 2) {
            dprintf(STDERR_FILENO,
                    "rm: Cannot remove %s, no such file or directory.\n", f);
        } else if (S_ISDIR(s.st_mode)) {
            dprintf(STDERR_FILENO,
                    "rm: %s is a directory.\n", f);
        } else if (status) {
            dprintf(STDERR_FILENO,
                    "rm: stat error: %d\n", status);
        } else {
            int n = -unlink(f);
            if (n) {
                dprintf(STDERR_FILENO,
                        "rm: error removing %s: %d\n", f, n);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: rm FILE...\n");
        return 0;
    }
    argv++;
    rm_file(argv);

    return 0;
}
