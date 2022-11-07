/* date = August 9th 2022 10:01 pm */

#ifndef __MEMORY_ARENA_H__
#define __MEMORY_ARENA_H__


#include "common.h"

#define KB(a) (a) * 1024
#define MB(a) KB(a) * 1024


typedef struct MemoryArenaBlock {
    u8* memory_start;
    u8* memory_end;
    struct MemoryArenaBlock* next;
} MemoryArenaBlock;


typedef struct MemoryArena {
    sz block_size;
    u8* current_adr;
    MemoryArenaBlock* current_block;
    MemoryArenaBlock* first_block;
    MemoryArenaBlock* last_block;
    u32 n_blocks;
} MemoryArena;


internal MemoryArena* memory_arena_create(sz memory_size);
internal void memory_arena_destroy(MemoryArena* arena);
internal void memory_arena_clear(MemoryArena* arena);
internal void* memory_arena_push(MemoryArena* arena, sz size); 


#endif //MEMORY_ARENA_H
