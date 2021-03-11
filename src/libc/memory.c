#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

typedef struct block_s block_s;

struct block_s {
	size_t size;
	bool free;
	struct block *next;
};

void* malloc(size_t size) {
	void* ptr = sbrk(size);
	if (ptr == (void*)-1)
		ptr = NULL;
	return ptr;
}

void free(void* ptr) {
	// stub
}
