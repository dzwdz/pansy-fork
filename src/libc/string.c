#include <stdint.h>
#include <string.h>
#include <unistd.h>

size_t strlen(const char *s) {
    size_t c = 0;
    while (*s++) c++;
    return c;
}

int strcmp(const char *s1, const char *s2) {
    do {
        if (*s1 != *s2++)
            return s1 - s2;
    } while (*s1++);

    return 0;
}

char *strcpy(char *dest, const char *src) {
    char *og = dest;
    while ((*dest++ = *src++));
    return og;
}

char *strcat(char *dest, const char *src) {
    size_t dest_len = strlen(dest);

    size_t i = 0;
    for (; src[i] != '\0'; i++)
        dest[dest_len + i] = src[i];
    dest[dest_len + i] = '\0';

    return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
    size_t dest_len = strlen(dest);
    size_t i = 0;

    for (; i < n && src[i] != '\0'; i++)
        dest[dest_len + i] = src[i];
    dest[dest_len + i] = '\0';

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *c1 = s1, *c2 = s2;
    for (int i = 0; i < n; i++) {
        if (c1[i] != c2[i]) {
            if (c1[i] < c2[i])  return -1;
            else                return 1;
        }
    }

    return 0;
}

// gcc does a galaxy brain move when optimizing our memset
// it just replaces it with a call to memset, which causes a stack overflow
// TODO just rewrite this in asm...
// also this isn't fully tested
void __attribute__((optimize("O1"))) *memset(void *dest, int cc, size_t n) {
    uint8_t c = cc;
    uint8_t *d = dest;
    uint8_t offset;

    // align *d with the reg size
    // i'm actually not quite sure if this is necessary
    // 64bit - 8 bytes - top 3 bits
    offset = (0b1000 - ((intptr_t)d & 0b111)) & 0b111;
    n -= offset;
    while (offset--)
        *d++ = c;

    // aight it's aligned
    // now let's save the amount of bytes at the end
    offset = n & 0b111;
    // and write in blocks
    uint64_t *dd = dest;
    uint64_t block = 0x0101010101010101 * c;
    n >>= 3;
    while (n-- > 0)
        *dd++ = block;

    // last bytes
    d = (void*)dd;
    while (offset--)
        *d++ = c;
    return dest;
}
