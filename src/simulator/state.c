#include "simulator/state.h"


internal State*
state_create() {
    State* state = memory_malloc(sizeof(*state), "state_create");
    check_memory(state);
    
    state->permanent_storage = memory_arena_create(BLOCK_SIZE);
    check(state->permanent_storage != NULL, "state->permanent_storage is NULL");
    
    state->transient_storage = memory_arena_create(BLOCK_SIZE);
    check(state->transient_storage != NULL, "state->transient_storage is NULL");
    
    error:
    
    if (state != NULL) {
        if (state->permanent_storage != NULL) memory_arena_destroy(state->permanent_storage);
        if (state->transient_storage != NULL) memory_arena_destroy(state->transient_storage);
        memory_free(state);
    }
    
    return NULL;
}


internal void
state_destroy(State* state) {
    check(state != NULL, "state is NULL");
    
    memory_arena_destroy(state->permanent_storage);
    memory_arena_destroy(state->transient_storage);
    memset(state, 0, sizeof(*state));
    memory_free(state);
    
    error:
    return;
}