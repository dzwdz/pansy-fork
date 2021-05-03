#include <stdio.h>
#include <stdbool.h>

// malloc.c
bool test_malloc();

// printf.c
bool test_sprintf();


// test runners are supposed to return true on success, false on failure
struct test_runner {
	char *name;
	bool (*fun)();
};

struct test_runner runners[] = {
	{"malloc", &test_malloc},
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
