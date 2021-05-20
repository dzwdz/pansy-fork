/* a terrible idea
 * will shit itself if you ever use multiple threads
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

// TODO measure peak memory usage
#define STACK_SIZE 10484576

static char *base = NULL;
static char *min  = NULL;
static char *top  = NULL;
static char *max  = NULL;

// NOT named after the anime
static void BNA_init() {
    if (base != NULL) return;
    base = top = min = malloc(STACK_SIZE);
    assert(base != NULL);
    max = base + STACK_SIZE;
}

void *BNA_alloc(size_t size) {
    top += size;
    assert(top <= max);
    return top - size;
}

void BNA_push() {
    if (base == NULL)
        BNA_init();

    *(void**)top = base;
    base = top;
    top += sizeof(void*);
}

void BNA_pop() {
    top = base;
    base = *(void**)base;
}
