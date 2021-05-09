#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

void readline(char* buf, size_t buf_size) {
    size_t i = 0;
    for (; i < buf_size; i++) {
        read(1, &buf[i], 1);
        if (buf[i] == '\n' || buf[i] == '\r') break;
    }
    buf[i] = '\0';
}

void hexdump(const void *buf, size_t len) {
    for (size_t i = 0; i < len; i += 16) {
        printf("%04d:", i);

        size_t j = 0;
        for (; j < 16 && i + j < len; j++) {
            printf(" %02x", ((uint8_t*)buf)[i + j]);
        }

        // align the plaintext column
        while (j < 16) {
            printf("   ");
            j++;
        }

        putchar('\t');

        for (j = 0; j < 16 && i + j < len; j++) {
            char c = ((uint8_t*)buf)[i + j];
            // ensure the character is printable
            if (!(' ' <= c && c <= '~')) c = '.';
            putchar(c);
        }
        putchar('\n');
    }
}

void clear_screen(void) {
    write(STDOUT_FILENO, "\033[2J\033[H", 7);
}
