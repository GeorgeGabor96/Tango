#ifndef __ENGINE_STATE_H__
#define __ENGINE_STATE_H__


typedef struct State {
    Memory* permanent_storage;
    Memory* transient_storage;
} State;


#define BLOCK_SIZE MB(10)


internal State* state_create();
internal void state_destroy(State* state);


#endif // __ENGINE_STATE_H__
