/* date = August 9th 2022 10:01 pm */

#ifndef __MEMORY_ARENA_H__
#define __MEMORY_ARENA_H__


#include "common.h"

typedef struct MemoryArena {
    sz size;
    sz current;
    u8* memory;
} MemoryArena;


internal MemoryArena* memory_arena_create(sz memory_size, u32 clear_to_zero);
internal void memory_arena_destroy(MemoryArena* arena);
internal void* memory_arena_push(MemoryArena* arena, sz size); 


#endif //MEMORY_ARENA_H