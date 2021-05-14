#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

union writer_arg {
    int fd;
    char *out_str;
};

typedef void (*writer_t)(union writer_arg, const char*, size_t);

// the writer abstracts away the writing part, so we can use the same function
// to write to strings and files
static int __vprintf_internal(writer_t writer, union writer_arg warg,
                             const char *fmt, va_list argp) {
    const char *sub = fmt;
    int total = 0;

    while (1) {
        unsigned int target_len = 0;
        bool long_option = false;

        char c = *fmt++;
        switch (c) {
        case '%':
            writer(warg, sub, fmt - sub - 1);
            total += fmt - sub - 1;

parse_fmt:
            c = *fmt++;
            switch (c) {
            // parsing modifiers
            case '0': {
                target_len = *fmt++ - '0';
                goto parse_fmt;}

            // printing stuff
            case 's': {
                const char *s = va_arg(argp, char*);

                // print leading zeros
                for (int k = target_len - strlen(s); k > 0; k--) {
                    char a = ' ';
                    writer(warg, &a, 1);
                }

                writer(warg, s, strlen(s));
                break;}
            case 'c': {
                char c = va_arg(argp, int);
                writer(warg, &c, 1);
                break;}
            case 'x': {
                unsigned int num = va_arg(argp, unsigned int);

                unsigned int i = 0;
                while (num >> i && i < (sizeof(int) * 8)) i += 4;
                if (i == 0) i = 4;
                if (target_len * 4 > i) i = target_len * 4;

                while (i > 0) {
                    i -= 4;
                    char c = '0' + ((num >> i) & 0xf);
                    if (c > '9') c += 'A' - '9' - 1;
                    writer(warg, &c, 1);
                }

                break;}
            case 'l': {
                // yeah yeah it's not good, but it works for now (doesn't handle ll)
                long_option = true;
                goto parse_fmt;
            }
            case 'd': {
                char c;
                /* ints can be converted to longs losslessly */
                long n;
                if (long_option) {
                    long_option = false;
                    n = va_arg(argp, long);
                } else {
                    n = va_arg(argp, int);
                }

                if (n < 0) {
                    c = '-';
                    writer(warg, &c, 1);
                    n = -n;
                    if (target_len > 0) target_len--;
                }

                // write to string
                char to_print[25] = {0};
                to_print[0] = '0'; // special case for n == 0
                int i = 0;
                while (n != 0) {
                    to_print[i++] = (n % 10) + '0';
                    n /= 10;
                }

                // print leading zeros
                for (int k = target_len - strlen(to_print); k > 0; k--) {
                    c = '0';
                    writer(warg, &c, 1);
                }

                // that previous string is reversed, so we print it backwards
                for (int k = strlen(to_print); k > 0; k--) {
                    c = to_print[k - 1];
                    writer(warg, &c, 1);
                }

                break;}
            }

            sub = fmt;
            break;

        case '\0':
            writer(warg, sub, fmt - sub);
            return total + (fmt - sub);
        }
    }
}


static void file_writer(union writer_arg warg, const char *buf, size_t len) {
    write(warg.fd, buf, len);
}

static void string_writer(union writer_arg warg, const char *buf, size_t len) {
    static size_t pos = 0;
    for (size_t i = 0; i < len; i++)
        warg.out_str[pos++] = buf[i];
    if (*buf == '\0') pos = 0;
}

int printf(const char *fmt, ...) {
    va_list argp;
    union writer_arg warg = {.fd = 1};

    va_start(argp, fmt);
    int printed = __vprintf_internal(file_writer, warg, fmt, argp);
    va_end(argp);

    return printed;
}

int dprintf(int fd, const char *fmt, ...) {
    va_list argp;
    union writer_arg warg = {.fd = fd};

    va_start(argp, fmt);
    int printed = __vprintf_internal(file_writer, warg, fmt, argp);
    va_end(argp);

    return printed;
}

int sprintf(char *out_str, const char *fmt, ...) {
    va_list argp;
    union writer_arg warg = {.out_str = out_str};

    va_start(argp, fmt);
    int printed = __vprintf_internal(string_writer, warg, fmt, argp);
    va_end(argp);

    return printed;
}
