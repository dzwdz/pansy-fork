/* a terrible idea
 * will shit itself if you ever use multiple threads
 * TODO adapt into a general purpose arena library
 */

#include <assert.h>
#include <bignum.h>
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

bignum BNA_newBN(uint16_t length) {
    return (bignum){
        .digits = BNA_alloc(length * sizeof(uint64_t)),
        .length = length
    };
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
