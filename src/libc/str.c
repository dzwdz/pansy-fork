#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <str.h>

mutstr mutstr_new(char *initial, size_t len) {
    size_t initial_len = strlen(initial);
    size_t newcap = initial_len * 2;

    mutstr ret = {
        .len = len,
        .capacity = newcap,
        .data = malloc(newcap),
    };

    if (initial != NULL && initial_len != 0)
        strcpy(ret.data, initial);
    return ret;
}


void mutstr_append(mutstr *dest, char *src) {
    size_t srclen = strlen(src);
    if (srclen + dest->len > dest->capacity) {
        // TODO: Expand the size of dest instead of just returning without
        // mutating anything. This probably should be done once realloc is
        // implemented.
        return;
    }

    strcpy(dest->data + dest->len, src);
    dest->len += srclen;
}

str from_cstr(const char *s) {
    str ret = {
        .len = strlen(s),
        .data = malloc(strlen(s) + 1),
    };

    /* Note: This copies over the 0-terminator, but it isn't used for anything
     * other than C-string interop. */
    strcpy(ret.data, s);

    return ret;
}

str from_mutstr(mutstr s) {
    str ret = {
        .len = s.len,
        .data = malloc(s.len + 1),
    };

    strcpy(ret.data, s.data);

    return ret;
}

bool str_equal(str a, str b) {
    if (a.len != b.len)
        return false;
    for (size_t i = 0; i < a.len; i++) {
        if (a.data[i] != b.data[i])
            return false;
    }

    return true;
}
