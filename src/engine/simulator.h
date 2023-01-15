#ifndef __ENGINE_SIMULATOR_H__
#define __ENGINE_SIMULATOR_H__


#define SIMULATOR_N_MAX_CALLBACKS 5u

typedef struct Simulator {
    Network* network;
    DataGen* data;
    Callback* callbacks[SIMULATOR_N_MAX_CALLBACKS];
    u32 n_callbacks;
} Simulator;


internal Simulator* simulator_create(State* state, Network* network, DataGen* data);
internal void simulator_infer(Simulator* simulator, State* state, ThreadPool* pool);
internal void simulator_learn(Simulator* simulator, State* state, ThreadPool* pool);
internal void simulator_add_callback(Simulator* simulator, State* state, Callback* callback);

#endif // __ENGINE_SIMULATOR_H__
