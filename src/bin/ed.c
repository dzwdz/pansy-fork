#include <tty.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct buffer {
    char **lines; // currently the line len is static == 256 TODO dynamic
    int line_amt; // amount of lines currently loaded
    int line_max; // size of lines[]

    char *path;
};

void buffer_append(struct buffer *buf, const char *from, int after) {
    int pos = after + 1;
    for (;;) {
        buf->line_amt++;
        char *c = malloc(256);
        memmove(&buf->lines[pos + 1],
                &buf->lines[pos],
                (buf->line_amt - pos) * sizeof(char*));
        buf->lines[pos++] = c;

        while (*from && *from != '\n') {
            *c++ = *from++;
        }
        *c = '\0';

        if (*from == '\0')
            break;

        from++; // skip the newline
    }
}

bool buffer_open(struct buffer *buf, const char *path) {
    // clean the buffer
    for (int i = 0; i < buf->line_amt; i++) {
        free(buf->lines[i]);
        buf->lines[i] = NULL;
    }
    buf->line_amt = 0;

    // read the file into memory
    int fd = open(path, 0);
    if (fd < 0) return false;

    off_t size = lseek(fd, 0, SEEK_END);
    if (size == 0) {
        close(fd);
        return true;
    }
    lseek(fd, 0, SEEK_SET);

    char *raw = malloc(size + 1);
    read(fd, raw, size);
    close(fd);
    raw[size] = '\0';


    buffer_append(buf, raw, -1);
    free(raw);
    return true;
}

bool buffer_write(struct buffer *buf) {
    int fd = open(buf->path, O_CREAT | O_WRONLY);
    if (fd < 0) return false;

    for (int i = 0; i < buf->line_amt; i++) {
        if (i != 0) write(fd, "\n", 1);
        write(fd, buf->lines[i], strlen(buf->lines[i]));
    }

    close(fd);
    return true;
}

bool parse_cmd(struct buffer *buf, const char *c) {
    const char *c_og = c;

    // bounds
    int low = 0, high = INT_MAX;
    while ('0' <= *c && *c <= '9')
        low = low * 10 + *c++ - '0';

    if (*c == ',') {
        c++;
        while ('0' <= *c && *c <= '9') {
            if (high == INT_MAX) high = 0;
            high = high * 10 + *c++ - '0';
        }
    } else if (c != c_og) {
        high = low;
    }

    if (high != INT_MAX) high++;
    if (high > buf->line_amt)
        high = buf->line_amt;

    // actual commands
    switch (*c++) {
        case '\0': // print w/ line numbers
            for (int i = low; i < high; i++)
                printf("%x\t%s\n", i, buf->lines[i]);
            break;
        case 'p': // print w/o line numbers
            for (int i = low; i < high; i++)
                printf("%s\n", buf->lines[i]);
            break;

        case 'a': { // append
            char *line_buf = malloc(256);
            int after = high - 1;
            for (;;) {
                readline(line_buf, 256);
                if (!strcmp(line_buf, "."))
                    break;
                buffer_append(buf, line_buf, after++);
            }
            break;}

        case 'w': { // write
            //if (*c++) {
            //    buf->path = c;
            //}
            buffer_write(buf);
            break;
        }
        case 'q': // quit
            exit(0);
            break; // compiler complains if this isn't here

        default:
            puts("unrecognized command");
            return false;
    }

    return true;
}

int main(int argc, char **argv) {
    // initialize the buffer
    struct buffer buf;
    buf.line_amt = 0;
    buf.line_max = 256;
    buf.lines = malloc(sizeof(char*) * buf.line_max);
    buf.path = "";

    if (argc > 1) {
        buf.path = argv[1];
        buffer_open(&buf, argv[1]);
    }

    // main loop
    const int cmd_buf_s = 256;
    char *cmd_buf = malloc(cmd_buf_s);

    for (;;) {
        printf("%s: ", buf.path);
        readline(cmd_buf, cmd_buf_s);
        if (!parse_cmd(&buf, cmd_buf))
            puts("?");
    }
}
