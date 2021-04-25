#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

/* clone of p (paginate) from plan9
 * usage: p [ -number ] [ file ...  ]
 * number specifies number of lines in a page
 */

int print_lines(int fd, int n) {
    char c;
    for (int i = 0; i < n; i++) {
        while (1) {
            if (read(fd, &c, 1) == 0)
                return EOF;
            if (c == '\n') {
                if (i == n-1)
                    return 1;
                i++;
            }
            putchar(c);
        }
    }

    return EOF;
}

// TODO: deal with reading from stdin if no file is provided
int main(int argc, char *argv[]) {
    int line_num = 22;

    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-') {
            line_num = -atoi(argv[i]);
        }
    }

    argv++;
    while (*argv) {
        if ((*argv)[0] == '-') { // not a file, ignore
            argv++;
            continue;
        } else {
            int fd = open(*argv, O_RDONLY);
            int status = 1;
            while (status != EOF) {
                status = print_lines(fd, line_num);
                char c;
                read(STDIN_FILENO, &c, 1);
            }

            close(fd);
            argv++;
        }
    }
}
