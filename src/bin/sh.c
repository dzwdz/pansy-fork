#include <tty.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_LEN 256

char** env;

char** split_args(const char* str) {
    static
    void*  shared_buf = NULL;
    if (!shared_buf)
           shared_buf = malloc(64 * sizeof(char*)
                             + MAX_LEN + 64);

    char** parts_start = shared_buf;
    char** parts       = parts_start;

    char* buf_start    = (char*)shared_buf + 64 * sizeof(char*);
    char* buf          = buf_start;

    bool quoted        = false;

    while (*str != '\0') {
        switch (*str) {
            // WHITESPACE
            case ' ':
            case '\n':
            case '\t':
                // if we're in a quote, just add the whitespace to the buffer
                if (quoted) {
                    *buf++ = *str;
                    break;
                }

                // ignore repeating whitespace
                if (buf == buf_start) break;

                *buf++ = '\0';
                *parts++ = buf_start;
                buf_start = buf;
                break;

            // QUOTES
            case '"':
                quoted ^= true;
                if (!quoted) {
                    *buf++ = '\0';
                    *parts++ = buf_start;
                    buf_start = buf;
                }
                break;

            // ESCAPED CHARS
            case '\\': {
                char c = *++str;
                switch (c) {
                    case 't': c = '\t'; break;
                    case 'r': c = '\r'; break;
                    case 'n': c = '\n'; break;
                    case '0': c = '\0'; break;
                }
                *buf++ = c;
                break;
            }

            default:
                *buf++ = *str;
        }
        str++;
    }

    if (buf != buf_start)  {
        *buf++ = '\0';
        *parts++ = buf_start;
    }

    *parts = NULL;
    return parts_start;
}

// this treats the path arg as a char[MAX_LEN], and it messes up stuff after
// the null byte
bool find_file(char* path) {
    struct stat sb;
    if (stat(path, &sb) == 0) return true;

    if (path[0] == '/') return false;

    // look in /bin
    memmove(path + 5, path, MAX_LEN - 5);
    memcpy(path, "/bin/", 5);
    return stat(path, &sb) == 0;
}

int run(char** args) {
    /* builtins */
    if (!strcmp(args[0], "exit")) {
        free(args);
        exit(0);
    }

    if (!strcmp(args[0], "cd")) {
        chdir(args[1]);
        return 0;
    }

    /* redirection stuff ahead */
    int save_out = dup(STDOUT_FILENO);
    int save_in = dup(STDIN_FILENO);

    int out = STDOUT_FILENO;
    int in = STDIN_FILENO;

    for (size_t i = 0; args[i] != NULL; i++) {
        if (args[i][0] == '>') {
            if (args[i+1] == NULL && strlen(args[i]) == 1) {
                puts("pansh: syntax error: unexpected newline");
                close(save_out);
                close(save_in);
                return 1;
            } else {
                /* we assume it's written `cmd >out` */
                char *filename = &args[i][1];
                /* if it's written `cmd > out`, fix the filename */
                if (strlen(args[i]) == 1)
                    filename = args[i+1];

                creat(filename, 0666);
                out = open(filename, O_WRONLY | O_TRUNC);
                args[i] = NULL;
                i++;
            }
        } else if (args[i][0] == '<') {
            if (args[i+1] == NULL && strlen(args[i]) == 1) {
                puts("pansh: syntax error: unexpected newline");
                close(save_out);
                close(save_in);
                return 1;
            } else {
                char *filename = &args[i][1];
                if (strlen(args[i]) == 1)
                    filename = args[i+1];

                in = open(filename, O_RDONLY);
                args[i] = NULL;
                i++;
            }
        }
    }

    // the command isn't a builtin, try running an executable
    char path[MAX_LEN];
    strcpy(path, args[0]);

    if (!find_file(path)) {
        puts("file not found");
        return 1;
    }

    pid_t p = fork();

    if (!p) {
        dup2(out, STDOUT_FILENO);
        dup2(in, STDIN_FILENO);
        int err = execve(path, args, env);
        printf("pansh: execve() error: %d\n", err);
        return 1;
    } else {
        close(save_out);
        close(save_in);

        int status;
        wait(&status);
        return status;
    }
}

int main(int argc __attribute__((unused)),
         char** argv __attribute__((unused)),
         char** envp) {

    env = envp;

    char buf[MAX_LEN];

    while (1) {
        getcwd(buf, MAX_LEN);
        printf("\t%s> ", buf);

        readline(buf, MAX_LEN);
        if (buf[0] == '\0')
            continue;

        char** args = split_args(buf);
        run(args);
        free(args);
    }

    return 0;
}
