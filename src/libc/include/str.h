#pragma once

#include <stdlib.h>

/* mutstr is a variable size, mutable string struct */
typedef struct {
    char *data;
    size_t len;
    size_t capacity;
} mutstr;

/* str is not mutable and is therefore safer. This makes it better for most operations */
typedef struct {
    char *data;
    size_t len;
} str;

mutstr mutstr_new(char *initial, size_t len);
void mutstr_append(mutstr *dest, char *src);
void mutstr_insert(mutstr *dest, char *src, size_t index);

#define to_cstr(s) ((s).data)
str from_cstr(const char *s);
str from_mutstr(mutstr s);
bool str_equal(str a, str b);
