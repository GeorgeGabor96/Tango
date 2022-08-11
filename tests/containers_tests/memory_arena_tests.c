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