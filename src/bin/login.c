#include "fs.h"
#include "tty.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    puts("\e[2J\033[H~ pansy linux ~");

    char *userbuf = malloc(256);
    strcpy(userbuf, "/Users/");
    char *passbuf = malloc(256);
    char *bonusbuf = malloc(256);

    while (1) {
        printf("login: ");
        readline(userbuf + 7, 128);

        if (!is_path_safe(userbuf)) {
            puts("illegal username");
            continue;
        }

        struct stat sb;
        if (stat(userbuf, &sb)) {
            puts("that user doesn't exist");
            continue;
        }

        printf("password: ");
        readline(passbuf, 256);

        char *suffix = userbuf + strlen(userbuf);

        { // password check
            strcpy(suffix, "/password");
            int fd = open(userbuf, 0);
            if (open(userbuf, 0) == -1) {
                puts("user has no password");
                continue;
            }

            bonusbuf[0] = '\0';
            int len = read(fd, bonusbuf, 255);
            close(fd);
            if (len < 0) {
                puts("couldn't read the password file");
                continue;
            }
            bonusbuf[len] = '\0';

            { // strip newlines
                char *iter = bonusbuf;
                while (*iter) {
                    if (*iter == '\n') {
                        *iter = '\0';
                        break;
                    }
                    iter++;
                }
            }

            if (strcmp(passbuf, bonusbuf)) {
                puts("wrong password");
                continue;
            }
        }

        // change the uid
        setreuid(sb.st_uid, sb.st_uid);
        setregid(sb.st_gid, sb.st_gid);

        // exec shell
        char *const sh[] = {"/bin/sh", NULL};
        execve(sh[0], sh, NULL);
        puts("couldn't launch the shell");
    }
}

/* todo
 * usernames containing /
 * usernames in [".", ".."]
 */
