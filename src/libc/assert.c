#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void __assert_fail(const char *expr, const char *file, int line, const char *func) {
    printf("\033[0;31massertion failed! \033[0m(%s)\n%s:%s at line %d\n", expr, file, func, line);
    exit(1); // TODO implement abort()
}
