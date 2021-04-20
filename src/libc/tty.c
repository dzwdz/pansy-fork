#include <stddef.h>
#include <unistd.h>

void readline(char* buf, size_t buf_size) {
    size_t i = 0;
    for (; i < buf_size; i++) {
        read(1, &buf[i], 1);
        if (buf[i] == '\n') break;
    }
    buf[i] = '\0';
}

