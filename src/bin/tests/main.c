#include <stdio.h>
#include <stdbool.h>

// malloc.c
bool test_malloc();

// bignums.c
bool test_bignum();

// hashes.c
bool test_sha256();

// printf.c
bool test_sprintf();


// test runners are supposed to return true on success, false on failure
struct test_runner {
    char *name;
    bool (*fun)();
};

struct test_runner runners[] = {
    {"malloc", &test_malloc},
    {"bignum", &test_bignum},
    {"sha256", &test_sha256},
    {"sprintf", &test_sprintf},
    {NULL, NULL}
};

int main() {
    struct test_runner *current = runners;

    while (current->name != NULL) {
        printf("    running %s...\n", current->name);
        if (!(*current->fun)()) {
            puts("failed!");
            return 1;
        }
        current++;
    }

    puts("all tests passed!");
}
