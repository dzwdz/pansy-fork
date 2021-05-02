#include <unistd.h>

int main() {
    write(STDOUT_FILENO, "\033[2J", 4);
    write(STDOUT_FILENO, "\033[H", 3);
}
