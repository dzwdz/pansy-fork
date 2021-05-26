#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    bool noption = false;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-n")) {
            noption = true;
        } else {
            printf("%s", argv[i]);
            if (i != argc - 1)
                putchar(' ');
        }
    }
    if (!noption)
        putchar('\n');
    return 0;
}
