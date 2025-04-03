#ifndef _MEMORY_ARENA_H
#define _MEMORY_ARENA_H

#include "common.h"

typedef struct {
    void  *pointer;
    size_t bytes;
    size_t cursor;
} Memory_Arena;

Memory_Arena mem_arena_alloc(size_t bytes);
void mem_arena_free(Memory_Arena *arena);
void *mem_arena_push(Memory_Arena *arena, size_t bytes);
void mem_arena_reset(Memory_Arena *arena);

#endif /* _MEMORY_ARENA_H */
