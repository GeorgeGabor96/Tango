#ifndef __UTILS_CONTAINERS_MEMORY_H__
#define __UTILS_CONTAINERS_MEMORY_H__


#define KB(a) (a) * 1024
#define MB(a) KB(a) * 1024


typedef struct MemoryBlock {
    u8* memory_start;
    u8* memory_end;
    struct MemoryBlock* next;
} MemoryBlock;


typedef struct Memory {
    sz block_size;
    u8* current_adr;
    MemoryBlock* current_block;
    MemoryBlock* first_block;
    MemoryBlock* last_block;
    u32 n_blocks;
    bool allow_bigger;
} Memory;


internal Memory* memory_create(sz memory_size, bool allow_bigger);
internal void memory_destroy(Memory* arena);
internal void memory_clear(Memory* arena);
internal void* memory_push(Memory* arena, sz size);


#endif // __UTILS_CONTAINERS_MEMORY_H__
