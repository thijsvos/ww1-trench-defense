#ifndef TD_MEMORY_H
#define TD_MEMORY_H

#include <stddef.h>
#include <stdint.h>

typedef struct Arena {
    uint8_t *buf;
    size_t size;
    size_t offset;
} Arena;

Arena arena_create(size_t size);
void *arena_alloc(Arena *a, size_t size, size_t align);
void arena_reset(Arena *a);
void arena_destroy(Arena *a);

/* Pool allocator - placeholder for future phases */
typedef struct Pool {
    uint8_t *buf;
    size_t block_size;
    size_t block_count;
    void *free_list;
} Pool;

#endif /* TD_MEMORY_H */
