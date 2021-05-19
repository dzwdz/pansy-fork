#include <assert.h>
#include <string.h>

const char *equal1  = "This string is equal to the other one."; // len = 38 without the null byte
const char *equal2  = "This string is equal to the other one.";
const char *partial = "This string isn't equal."; // len = 24

void test_string() {
    assert(strcmp(equal1, equal2)   == 0);
    assert(strcmp(equal1, partial)  != 0);

    assert(memcmp(equal1, equal2, 39)   == 0);
    assert(memcmp(equal1, partial, 14)  == 0);
    assert(memcmp(equal1, partial, 15)  != 0);
}
