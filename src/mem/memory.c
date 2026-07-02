#include "allocator.h"

#include <stdlib.h>

void _free(void *const mem) {
    if (mem == NULL) return;
    free(mem);
}

void *_malloc(const size_t size) {
    return malloc(size);
}