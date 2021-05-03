#include <stdbool.h>
#include <string.h>
#include <stdio.h>

bool test_sprintf() {
    char buf[256];
    sprintf(buf, "%s %c %x %d %03d test", "foo", 'c', 42, 42, 1);
    return !strcmp("foo c 2A 42 001 test", buf);
}
