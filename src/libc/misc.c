#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <unistd.h>

void __libc_start_main(int argc, char** argv,
        int (*main)(int, char**, char**)) {
    char **envp = argv+argc+1;

    exit(main(argc, argv, envp));
}

int putchar(int c) {
    return write(1, &c, 1);
}

int puts(const char *s) {
    int b = write(1, s, strlen(s));
    if (b < 0) return b;
    return b + write(1, "\n", 1);
}


bool is_number(const char *str) {
    if ((*str == '-') && (strlen(str) == 1))
        return false;
    for (size_t i = *str == '-' ? 1 : 0; i < strlen(str); i++) {
        if (!(str[i] >= '0' && str[i] <= '9'))
            return false;
    }
    return true;
}

int atoi(const char *str) {
    if (!is_number(str))
        return 0;
    int n = 0;
    int sign = 1;
    int i = 0;
    if (str[i] == '-') {
        sign = -1;
        i++;
    }
    for (; str[i] != '\0'; i++) {
        n = n * 10 + str[i] - '0';
    }
    return n * sign;
}

// deviates from the spec - accepts any length
int getentropy(void *buffer, size_t len) {
    char *pos = buffer;

    while (len) {
        int ret = getrandom(pos, len, 0);
        if (ret < 0)  {
            // TODO check for interrupts
            return ret;
        }
        pos += ret;
        len -= ret;
    }

    return 0;
}
