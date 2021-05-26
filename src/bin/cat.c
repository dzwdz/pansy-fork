#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void cat(int fd) {
    char c;
    while (read(fd, &c, 1)) {
        putchar(c);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cat(STDIN_FILENO);
        return 1;
    }
    argv++; // we don't care about program name

    for (; *argv != NULL; argv++) {
        int file;
        if (!strcmp(*argv, "-"))
            file = STDIN_FILENO;
        else
            file = open(*argv, O_RDONLY);
        if (file < 0) {
            // TODO: use errno to give helpful error messages
            dprintf(STDERR_FILENO,
                    "cat: An error occurred while opening %s.\n", *argv);
        } else {
            cat(file);
        }

        if (file != STDIN_FILENO)
            close(file);
    }

    return 0;
}
