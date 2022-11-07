#include "tests.h"
#include "containers/memory_arena.c"


internal TestStatus
memory_arena_create_destroy_test() {
    TestStatus status = TEST_FAILED;
    sz block_size = MB(11);
    MemoryArena* arena = NULL;
    
    arena = memory_arena_create(block_size);
    assert(arena != NULL, "arena is NULL");
    assert(arena->block_size == block_size, "arena->block_size is %zu not %zu",
           arena->block_size, block_size);
    assert(arena->current_block != NULL, "arena->current_block is NULL");
    assert(arena->first_block != NULL, "arena->first_block is NULL");
    assert(arena->first_block->next == NULL, "arena->first_block->next is %p not NULL",
           arena->first_block->next);
    assert(arena->last_block != NULL, "arena->last_block is NULL");
    assert(arena->current_adr == arena->first_block->memory_start,
           "arena->current_adr is %p not arena->first_block->memory_start %p",
           arena->current_adr, arena->first_block->memory_start);
    assert(arena->first_block == arena->last_block,
           "arena->first_block is %p, arena->last_blocks is %p",
           arena->first_block, arena->last_block);
    block_size = arena->first_block->memory_end - arena->first_block->memory_start; 
    assert(block_size == arena->block_size, "block size is %zu not %zu",
           block_size, arena->block_size);
    assert(arena->n_blocks == 1, "arena->n_blocks is %u not 1", arena->n_blocks);
    memory_arena_destroy(arena);
    
    // EDGE cases
    arena = memory_arena_create(0);
    assert(arena == NULL, "should return NULL for size 0");
    
    memory_arena_destroy(NULL);
    
    assert(memory_leak() == FALSE, "Memory Leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
memory_arena_push_test() {
    TestStatus status = TEST_FAILED;
    MemoryArena* arena = memory_arena_create(MB(10));
    sz size1 = MB(1);
    sz size2 = KB(10);
    sz size3 = MB(6);
    void* pointer = NULL;
    u8* true_pointer = NULL;
    
    true_pointer = arena->current_adr;
    pointer = memory_arena_push(arena, size1);
    assert(pointer == true_pointer,
           "returned pointer should be %p not %p",
           true_pointer, pointer);
    assert(arena->n_blocks == 1, "arena->n_blocks is %u not 1", arena->n_blocks);
    
    pointer = memory_arena_push(arena, size2);
    true_pointer += size1;
    assert(pointer == true_pointer,
           "returned pointer should be %p not %p",
           true_pointer, pointer);
    assert(arena->n_blocks == 1, "arena->n_blocks is %u not 1", arena->n_blocks);
    
    pointer = memory_arena_push(arena, size3);
    true_pointer += size2;
    assert(pointer == true_pointer,
           "returned pointer should be %p not %p",
           true_pointer, pointer);
    assert(arena->n_blocks == 1, "arena->n_blocks is %u not 1", arena->n_blocks);
    
    pointer = memory_arena_push(arena, size3);
    true_pointer = arena->last_block->memory_start;
    assert(pointer == true_pointer, "pointer should be %p", true_pointer);
    assert(arena->n_blocks == 2, "arena->n_blocks is %u not 2", arena->n_blocks);
    assert(arena->current_block == arena->last_block,
           "arena->current_block is %p arena->last_block is %p",
           arena->current_block, arena->last_block);
    assert(arena->first_block != arena->last_block,
           "arena->first_block == arena->last_block");
    
    pointer = memory_arena_push(arena, size1);
    true_pointer += size3;
    assert(pointer == true_pointer,
           "returned pointer should be %p not %p",
           true_pointer, pointer);
    assert(arena->n_blocks == 2, "arena->n_blocks is %u not 2", arena->n_blocks);
    
    pointer = memory_arena_push(arena, MB(10));
    true_pointer = arena->last_block->memory_start;
    assert(pointer == true_pointer, "pointer should be %p not %p", true_pointer, pointer);
    assert(arena->n_blocks == 3, "arena should have 3 blocks not %u", arena->n_blocks);
    assert(arena->current_block == arena->last_block,
           "arena->current_block is %p arena->last_block is %p",
           arena->current_block, arena->last_block);
    assert(arena->first_block != arena->last_block,
           "arena->first_block == arena->last_block");
    
    // EDGE cases
    pointer = memory_arena_push(NULL, size1);
    assert(pointer == NULL, "pointer should be NULL for NULL arena");
    pointer = memory_arena_push(arena, 0);
    assert(pointer == NULL, "pointer should be NULL for size of 0");
    pointer = memory_arena_push(arena, MB(11));
    assert(pointer == NULL, "pointer should be nULL for size > memory_blocks");
    
    memory_arena_destroy(arena);
    assert(memory_leak() == FALSE, "Memory Leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
memory_arena_alignment_test() {
    TestStatus status = TEST_FAILED;
    MemoryArena* arena = memory_arena_create(MB(1));
    
    /*
NOTE: What happends if I want 1 byte and them 4 bytes or 8 bytes, the memory will not
be alligned anymore write? or at least it depends on the type but in general
I store an u8 and after that a pointer, will it break because the pointer is not 
alignmed to sizeof(void*)?
*/
    
    u8* byte_p = memory_arena_push(arena, 1);
    u32* numbers_p = memory_arena_push(arena, sizeof(u32) * 5);
    
    // NOTE: THe allocation should return a pointer alignmed to sizeof(void*)
    assert((sz)arena % sizeof(void*) == 0, "arena should be alinged to sizeof(void*)");
    assert((sz)byte_p % sizeof(void*) == 0, "byte_p should be aligned");
    assert((sz)numbers_p % sizeof(void*) == 0, "numbers_p should be aligned");
    
    *byte_p = 1;
    numbers_p[0] = 1;
    numbers_p[1] = 2;
    numbers_p[2] = 3;
    numbers_p[3] = 4;
    numbers_p[4] = 5;
    
    memory_arena_destroy(arena);
    
    arena = memory_arena_create(MB(1));
    byte_p = memory_arena_push(arena, 1);
    
    u32** adr = memory_arena_push(arena, sizeof(void*));
    assert((sz)adr % sizeof(void*) == 0, "adr should be aligned");
    
    *adr = memory_arena_push(arena, sizeof(u32*) * 10);
    assert((sz)(*adr) % sizeof(void*) == 0, "*adr should be aligned");
    (*adr)[0] = 0;
    (*adr)[1] = 10;
    (*adr)[2] = 20;
    (*adr)[3] = 30;
    
    memory_arena_destroy(arena);
    
    assert(memory_leak() == FALSE, "Memory Leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
memory_arena_clear_test() {
    TestStatus status = TEST_FAILED;
    
    MemoryArena* arena = memory_arena_create(MB(1));
    u8* p = memory_arena_push(arena, MB(1));
    p = memory_arena_push(arena, MB(1));
    p = memory_arena_push(arena, MB(1));
    
    memory_arena_clear(arena);
    assert(arena->first_block != arena->last_block,
           "arena->first_block == arena->last_block");
    assert(arena->current_block == arena->first_block,
           "arena->current_block is %p arena->first_block is %p",
           arena->current_block, arena->first_block);
    assert(arena->n_blocks == 3, "arena->n_blocks is %u not 3", arena->n_blocks);
    assert(arena->current_adr == arena->current_block->memory_start,
           "arena->current_adr %p != arena->current_block->memory_start %p",
           arena->current_adr, arena->current_block->memory_start);
    assert(arena->first_block->next != NULL, "arena->first_block->next is NULL");
    assert(arena->last_block->next == NULL, "arena->last_block->next is %p not NULL",
           arena->last_block->next);
    
    memory_arena_destroy(arena);
    
    assert(memory_leak() == FALSE, "Memory Leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}