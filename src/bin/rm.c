#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

// TODO: print errors stderr
void rm_file(char **files) {
    struct stat s;
    char *f;

    while ((f = *files++) != NULL) {
        int status = stat(f, &s);
        if (status == -2) {
            printf("rm: Cannot remove %s, no such file or directory.\n", f);
        } else if (S_ISDIR(s.st_mode)) {
            printf("rm: %s is a directory.\n", f);
        } else if (status) {
            printf("rm: stat error: %d\n", status);
        } else {
            int n = -unlink(f);
            if (n) {
                printf("rm: error removing %s: %d\n", f, n);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2)
        return 0;
    argv++;
    rm_file(argv);
}
