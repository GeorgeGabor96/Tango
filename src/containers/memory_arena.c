#include "containers/memory_arena.h"


internal MemoryArena*
memory_arena_create(sz memory_size, bool clear_to_zero) {
    MemoryArena* arena = NULL;
    
    check(memory_size != 0, "memory_size is 0");
    // NOTE: so we need memory_size bytes to be referred in the memory
    // NOTE: field so add the arena size to that
    if (clear_to_zero == TRUE) {
        arena = (MemoryArena*)memory_calloc(memory_size + sizeof(MemoryArena), 
                                            1,
                                            "memory_arena_create with calloc");
    } else {
        arena = (MemoryArena*)memory_malloc(memory_size + sizeof(MemoryArena),
                                            "memory_arena_create with malloc");
    }
    check_memory(arena);
    arena->size = memory_size;
    arena->current = 0;
    arena->memory = (u8*)(arena + 1);
    
    error:
    return arena;
}


internal void
memory_arena_destroy(MemoryArena* arena) {
    check(arena != NULL, "arena is NULL");
    
    arena->size = 0;
    arena->current = 0;
    arena->memory = NULL;
    memory_free(arena);
    
    error:
    return;
}


internal void*
memory_arena_push(MemoryArena* arena, sz size) {
    u8* memory = NULL;
    check(arena != NULL, "arena is NULL");
    check(size != 0, "size is 0");
    
    // NOTE: On this machine if memory is not aligned based on the type, its okay, but
    // NOTE: pretty sure that the CPU is doing more that it should
    // NOTE: for this, aligned everything to sizeof(void*) so that there is no problem
    sz reminder = size % sizeof(void*);
    if (reminder != 0) {
        size += sizeof(void*) - reminder;
    }
    
    // TODO: should it be with =?
    if (arena->current + size > arena->size) {
        log_warning("The memory arena cannot hold another %zu bytes", size);
    } else {
        memory = arena->memory + arena->current;
        arena->current += size;
    }
    
    error:
    return memory; 
}
