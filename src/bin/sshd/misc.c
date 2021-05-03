#include "misc.h"
#include <stdlib.h>
#include <string.h>

iter_t iterator_copy(const iter_t *orig) {
    iter_t copy;
    copy.base = malloc(orig->max);
    copy.max = orig->max;
    copy.pos = orig->pos;
    memcpy(copy.base, orig->base, copy.max);
    return copy;
}
