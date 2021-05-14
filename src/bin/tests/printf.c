#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

void test_sprintf() {
    char buf[256];
    sprintf(buf, "%s %c %x %d %03d test", "foo", 'c', 42, 42, 1);
    assert(strcmp("foo c 2A 42 001 test", buf) == 0);
}
