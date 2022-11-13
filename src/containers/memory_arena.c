#include "containers/memory_arena.h"


inline internal void
memory_arena_add_block(MemoryArena* arena) {
    MemoryArenaBlock* block = NULL;
    check(arena != NULL, "arena is NULL");
    
    block = (MemoryArenaBlock*)
        memory_malloc(sizeof(*block) + arena->block_size,
                      "memory_arena_block_create");
    check_memory(block);
    
    block->memory_start = (u8*)(block + 1);
    block->memory_end = block->memory_start + arena->block_size;
    block->next = NULL;
    
    if (arena->first_block == NULL) {
        arena->first_block = block;
        arena->last_block = block;
    } else {
        arena->last_block->next = block;
        arena->last_block = block;
    }
    
    arena->current_block = block;
    ++(arena->n_blocks);
    arena->current_adr = arena->current_block->memory_start;
    
    error:
    return;
}


internal MemoryArena*
memory_arena_create(sz block_size) {
    MemoryArena* arena = NULL;
    MemoryArenaBlock* block = NULL;
    
    check(block_size != 0, "block_size is 0");
    arena = (MemoryArena*) memory_calloc(1, sizeof(*arena), "memory_arena_create");
    check_memory(arena);
    arena->block_size = block_size;
    
    memory_arena_add_block(arena);
    
    return arena;
    
    error:
    if (arena != NULL) {
        if (arena->first_block != NULL) memory_free(arena->first_block);
        memory_free(arena);
    }
    
    return NULL;
}


internal void
memory_arena_destroy(MemoryArena* arena) {
    check(arena != NULL, "arena is NULL");
    
    MemoryArenaBlock* it = arena->first_block;
    MemoryArenaBlock* aux = NULL;
    
    while (it != NULL) {
        aux = it;
        it = it->next;
        memset(aux, 0, sizeof(*aux) + arena->block_size);
        memory_free(aux);
    }
    memset(arena, 0, sizeof(*arena));
    memory_free(arena);
    
    error:
    return;
}


internal void
memory_arena_clear(MemoryArena* arena) {
    check(arena != NULL, "arena is NULL");
    
    arena->current_block = arena->first_block;
    arena->current_adr = arena->first_block->memory_start;
    
    error:
    return;
}



internal void*
memory_arena_push(MemoryArena* arena, sz size) {
    u8* memory = NULL;
    
    check(arena != NULL, "arena is NULL");
    check(size != 0, "size is 0");
    check(size <= arena->block_size,
          "size %zu is bigger than the block size %zu",
          size, arena->block_size);
    // NOTE: We could like allocate into multiple block but discontinuities in memory are 
    // hard to work with, for now just increase the block size when this happens
    
    // NOTE: On this machine if memory is not aligned based on the type, its okay, but
    // NOTE: pretty sure that the CPU is doing more that it should
    // NOTE: for this, aligned everything to sizeof(void*) so that there is no problem
    
    // TODO: we could make this better by considering types that are lower than the size_t
    // TODO: lets see if it will make sense to do this
    sz reminder = size % sizeof(void*);
    if (reminder != 0) {
        size += sizeof(void*) - reminder;
    }
    
    // NOTE: check if it fits in current block
    if (arena->current_adr + size <= arena->current_block->memory_end) {
        memory = arena->current_adr;
        arena->current_adr += size;
    } else {
        // NOTE: check if the current block has a next and use that
        if (arena->current_block->next != NULL) {
            arena->current_block = arena->current_block->next;
        } else {
            memory_arena_add_block(arena);
        }
        
        memory = arena->current_block->memory_start;
        arena->current_adr = arena->current_block->memory_start + size;
    }
    
    error:
    return (void*)memory; 
}
