// just a small program i used for testing the shell

#include <stdio.h>

int main(int argc, char** argv, char** envp) {
    puts("arguments:");
    for (int i = 0; i < argc; i++)
        puts(argv[i]);

    puts("env:");
    while(*envp)
        puts(*envp++);

    return 0;
}
