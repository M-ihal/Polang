#include "memory_arena.h"

Memory_Arena mem_arena_alloc(size_t bytes) {
    return (Memory_Arena) {
        .pointer = malloc(bytes),
        .bytes = bytes,
        .cursor = 0
    };
}

void mem_arena_free(Memory_Arena *arena) {
    if(arena->pointer != NULL) {
        free(arena->pointer);
        *arena = (Memory_Arena) { };
    }
}

void *mem_arena_push(Memory_Arena *arena, size_t bytes) {
    const size_t rem = arena->bytes - arena->cursor;
    if(rem < bytes) {
        return NULL;
    }

    void *pointer = arena->pointer + arena->cursor;
    arena->cursor += bytes;
    return pointer;
}

void mem_arena_reset(Memory_Arena *arena) {
    arena->cursor = 0;
}
