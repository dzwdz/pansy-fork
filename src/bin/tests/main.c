#include <stdio.h>
#include <stdbool.h>

// malloc.c
void test_malloc();

// bignums.c
bool test_bignum();

// hashes.c
bool test_sha256();

// printf.c
void test_sprintf();

// string.c
void test_string();


// test runners are supposed to return true on success, false on failure
struct test_runner {
    char *name;
    void (*fun)();
};

struct test_runner runners[] = {
    {"malloc", &test_malloc},
    {"sprintf", &test_sprintf},
    {"string", &test_string},
    {"bignum", &test_bignum},
    {"sha256", &test_sha256},
    {NULL, NULL}
};

int main() {
    struct test_runner *current = runners;

    while (current->name != NULL) {
        printf("    running %s...\n", current->name);
        (*current->fun)();
        current++;
    }

    puts("all tests passed!");
}
