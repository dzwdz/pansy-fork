#include <stdbool.h>
#include <stddef.h>
//#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

/*** SYSCALLS ***/
void *mmap(void *addr, size_t len, int prot, int flags,
           int fildes, off_t off) {
    return (void*) syscall(SYS_mmap, addr, len, prot, flags, fildes, off);
}

void *sbrk(intptr_t increment) {
    void* brk = (void*) syscall(SYS_brk, 0);
    void* ret = (void*) syscall(SYS_brk, brk + increment);
    if (brk + increment != ret) return (void*)-1;
    return brk;
}


/*** MALLOC ***/
typedef struct block_s block_s;

struct block_s {
    size_t size;
    bool free;
    struct block *next;
};

void* malloc(size_t size) {
//    printf("allocating %x...\n", (unsigned int)size);
    void* ptr = sbrk(sizeof(block_s) + size);
    if (ptr == (void*)-1)
        return NULL;

    block_s *b = (block_s*)ptr;
    b->size = size;
    b->free = false;
    b->next = NULL; // we don't deal with these yet for free()
//    printf("allocated %x\n", (unsigned int)ptr);
//    printf("to        %x\n", (unsigned int)ptr+size);
    return ptr + sizeof(block_s);
}

void free(void* ptr) {
    if (ptr == NULL)
        return;

    block_s *b = (block_s*)(ptr - sizeof(block_s));
    b->free = true;
}


/*** MEMCPY n others ***/

// note: no overlap check
void *memcpy(void *dest, const void *src, size_t n) {
    char *d2 = dest;
    const char *s2 = src;
    while (n--) {
        *d2++ = *s2++;
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    char *tmp = malloc(n);
    if (tmp == NULL) return NULL;

    memcpy(tmp,  src, n);
    memcpy(dest, tmp, n);

    free(tmp);
    return dest;
}
