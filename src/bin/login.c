#include <fs.h>
#include <tty.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

enum id_value {
    UID,
    GID
};

uid_t read_id(char *path, enum id_value id) {
    int fd = open(path, O_RDONLY);

    char buf[10] = {0};

    char c;
    unsigned int current_line = 0;
    size_t i = 0;
    bool finished = false;
    while (!finished) {
        read(fd, &c, 1);
        if (current_line == id)
            buf[i++] = c;
        if (c == '\n') {
            if (current_line == id)
                finished = true;
            else
                current_line++;
        }
    };

    // remove the newline
    buf[i-1] = '\0';

    close(fd);
    return atoi(buf);
}

int main() {
    puts("\e[2J\033[H~ pansy linux ~");

    char *userbuf = malloc(256);
    strcpy(userbuf, "/Users/");
    char *passbuf = malloc(256);
    char *bonusbuf = malloc(256);
    char username[128] = {0}; // used later for uid checking

    while (1) {
        printf("login: ");
        readline(username, 128);
        strcpy(userbuf + 7, username);

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

        char uid_path[256];
        sprintf(uid_path, "/Users/%s/uid", username);
        uid_t uid = read_id(uid_path, UID);
        gid_t gid = read_id(uid_path, GID);

        // change the uid
        setreuid(uid, uid);
        setregid(gid, gid);

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
