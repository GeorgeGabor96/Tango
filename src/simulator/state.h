/* date = November 7th 2022 9:59 pm */

#ifndef STATE_H
#define STATE_H


#include "common.h"
#include "containers/memory_arena.h"


typedef struct State {
    MemoryArena* permanent_storage;
    MemoryArena* transient_storage;
} State;


#define BLOCK_SIZE MB(10)


internal State* state_create();
internal void state_destroy(State* state);


#endif //STATE_H
