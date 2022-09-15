#include "tests.h"
#include "containers/memory_arena.c"


internal TestStatus
memory_arena_create_destroy_test() {
    TestStatus status = TEST_FAILED;
    sz size = MB(10);
    MemoryArena* arena = NULL;
    
    arena = memory_arena_create(size, TRUE);
    assert(arena != NULL, "arena is NULL");
    assert(arena->size == size, "arena->size is %zu not %zu",
           arena->size, size);
    assert(arena->current == 0, "arena->current is %zu not 0",
           arena->current);
    assert(arena->memory != NULL, "arena->memory is %p not NULL", 
           arena->memory);
    for (sz idx = 0; idx < arena->size; ++idx)
        assert(arena->memory[idx] == 0,
               "arena->memory[%zu] is %u not 0",
               idx, arena->memory[idx]);
    memory_arena_destroy(arena);
    
    arena = memory_arena_create(size, FALSE);
    assert(arena != NULL, "arena is NULL");
    assert(arena->size == size, "arena->size is %zu not %zu",
           arena->size, size);
    assert(arena->current == 0, "arena->current is %zu not 0",
           arena->current);
    assert(arena->memory != NULL, "arena->memory is %p not NULL", 
           arena->memory);
    // NOTE: here we don't care about the values in the memory
    memory_arena_destroy(arena);
    
    // EDGE cases
    arena = memory_arena_create(0, FALSE);
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
    MemoryArena* arena = memory_arena_create(MB(10), TRUE);
    sz size1 = MB(1);
    sz size2 = KB(10);
    sz size3 = MB(6);
    void* pointer = NULL;
    u8* true_pointer = NULL;
    
    pointer = memory_arena_push(arena, size1);
    true_pointer = arena->memory;
    assert(pointer == true_pointer,
           "returned pointer should be %p not %p",
           true_pointer, pointer);
    
    pointer = memory_arena_push(arena, size2);
    true_pointer += size1;
    assert(pointer == true_pointer,
           "returned pointer should be %p not %p",
           true_pointer, pointer);
    
    pointer = memory_arena_push(arena, size3);
    true_pointer += size2;
    assert(pointer == true_pointer,
           "returned pointer should be %p not %p",
           true_pointer, pointer);
    
    pointer = memory_arena_push(arena, size3);
    // NOTE: this should be too big
    assert(pointer == NULL, "pointer should be NULL not %p",
           pointer);
    
    pointer = memory_arena_push(arena, size1);
    // NOTE: this should work
    true_pointer += size3;
    assert(pointer == true_pointer,
           "returned pointer should be %p not %p",
           true_pointer, pointer);
    
    // EDGE cases
    pointer = memory_arena_push(NULL, size1);
    assert(pointer == NULL, "pointer should be NULL for NULL arena");
    pointer = memory_arena_push(arena, 0);
    assert(pointer == NULL, "pointer should be NULL for size of 0");
    
    memory_arena_destroy(arena);
    assert(memory_leak() == FALSE, "Memory Leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
memory_arena_alignment_test() {
    TestStatus status = TEST_FAILED;
    MemoryArena* arena = memory_arena_create(MB(1), TRUE);
    
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
    
    arena = memory_arena_create(MB(1), TRUE);
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