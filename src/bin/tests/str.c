#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <str.h>

// BE WARY! According to gdb, some of these str's gets optimized out at
// compilation. These are tested properly with -O0 though, so it shouldn't be a
// problem. Just be aware of this when adding new tests.
void test_str() {
    str str1 = from_cstr("foobar");
    // do it this way since "foobar" will null-terminate it
    char *test_str = "foobar";
    assert(!strcmp(to_cstr(str1), test_str));

    str str2 = from_cstr("foobar");
    assert(str_equal(str1, str2));

    str str3 = from_cstr("barbaz");
    assert(!str_equal(str1, str3));

    str str4 = from_cstr("aa");
    assert(!str_equal(str1, str4));

    mutstr mutable = mutstr_new("foo", 3);
    mutstr_append(&mutable, "bar");
    str str5 = from_mutstr(mutable);
    assert(str_equal(str5, str1));

    mutstr mutable2 = mutstr_new("foobar", 6);
    mutstr_append(&mutable2, "baz");
    str str6 = from_mutstr(mutable2);
    str str7 = from_cstr("foobarbaz");
    assert(str_equal(str6, str7));

    mutstr_append(&mutable2, "qux");
    str str8 = from_mutstr(mutable2);
    str str9 = from_cstr("foobarbazqux");
    assert(str_equal(str8, str9));

    mutstr mutable3 = mutstr_new("foobar", strlen("foobar"));
    str str10 = from_cstr("baz");
    mutstr_append(&mutable3, to_cstr(str10));
    str str11 = from_mutstr(mutable3);
    assert(str_equal(str11, str7));

    mutstr mutable4 = mutstr_new("foobar", strlen("foobar"));
    mutstr_insert(&mutable4, "a", 1);
    str str12 = from_mutstr(mutable4);
    str str13 = from_cstr("faoobar");
    assert(str_equal(str12, str13));
}
