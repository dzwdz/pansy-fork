#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static const int sizes[] = {
    2, 100, 2, 10, 100, 10, 2, 23232
};

static const int size_amt = 8;

void test_malloc() {
    char **ptrs = malloc(size_amt * sizeof(char *));

    // first we allocate the memory blocks and fill them with data
    for (int i = 0; i < size_amt; i++) {
        int size = sizes[i];
        ptrs[i] = malloc(size);

        memset(ptrs[i], i, size);
    }

    // then we check if they didn't get corrupted
    for (int i = 0; i < size_amt; i++) {
        int size = sizes[i];
        for (int j = 0; j < size; j++) {
            assert(ptrs[i][j] == i);
        }
    }

    // they didn't, let's free them
    for (int i = 0; i < size_amt; i++) {
        free(ptrs[i]);
    }
}
