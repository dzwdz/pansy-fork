#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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

int printf(const char *fmt, ...) {
    const char *sub = fmt;
    int total = 0;

    va_list argp;
    va_start(argp, fmt);

    while (1) {
        int target_len = 0;

        char c = *fmt++;
        switch (c) {
        case '%':
            write(1, sub, fmt - sub - 1);
            total += fmt - sub - 1;

parse_fmt:
            c = *fmt++;
            switch (c) {
            case 's': {
                const char *s = va_arg(argp, char*);

                // print leading zeros
                for (int k = target_len - strlen(s); k > 0; k--) {
                    char a = ' ';
                    write(1, &a, 1);
                }

                write(1, s, strlen(s));
                break;}
            case 'c': {
                char c = va_arg(argp, int);
                write(1, &c, 1);
                break;}
            case 'x': {
                unsigned int num = va_arg(argp, unsigned int);

                int i = 0;
                while (num >> i && i < (sizeof(int) * 8)) i += 4;
                if (i == 0) i = 4;
                if (target_len * 4 > i) i = target_len * 4;

                while (i > 0) {
                    i -= 4;
                    char c = '0' + ((num >> i) & 0xf);
                    if (c > '9') c += 'A' - '9' - 1;
                    write(1, &c, 1);
                }

                break;}
            case '0': {
                target_len = *fmt++ - '0';
                goto parse_fmt;}
            case 'd': {
                char c;
                int n = va_arg(argp, int);

                if (n < 0) {
                    c = '-';
                    write(1, &c, 1);
                    n = -n;
                    if (target_len > 0) target_len--;
                }

                // write to string
                char to_print[10] = {0};
                to_print[0] = '0'; // special case for n == 0
                int i = 0;
                while (n != 0) {
                    to_print[i++] = (n % 10) + '0';
                    n /= 10;
                }

                // print leading zeros
                for (int k = target_len - strlen(to_print); k > 0; k--) {
                    c = '0';
                    write(1, &c, 1);
                }

                // that previous string is reversed, so we print it backwards
                for (int k = strlen(to_print); k > 0; k--) {
                    c = to_print[k - 1];
                    write(1, &c, 1);
                }

                break;}
            }

            sub = fmt;
            break;

        case '\0':
            write(1, sub, fmt - sub);
            return total + (fmt - sub);
        }
    }

    va_end(argp);
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
