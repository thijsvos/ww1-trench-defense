#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

Arena arena_create(size_t size) {
    Arena a;
    a.buf = (uint8_t *)malloc(size);
    a.size = size;
    a.offset = 0;
    if (!a.buf) {
        LOG_ERROR("Failed to allocate arena of size %zu", size);
        a.size = 0;
    }
    return a;
}

void *arena_alloc(Arena *a, size_t size, size_t align) {
    /* Round offset up to alignment boundary */
    size_t aligned_offset = (a->offset + (align - 1)) & ~(align - 1);

    if (aligned_offset + size > a->size) {
        LOG_ERROR("Arena out of memory: requested %zu, available %zu",
                  size, a->size - aligned_offset);
        return NULL;
    }

    void *ptr = a->buf + aligned_offset;
    a->offset = aligned_offset + size;
    memset(ptr, 0, size);
    return ptr;
}

void arena_reset(Arena *a) {
    a->offset = 0;
}

void arena_destroy(Arena *a) {
    free(a->buf);
    a->buf = NULL;
    a->size = 0;
    a->offset = 0;
}
