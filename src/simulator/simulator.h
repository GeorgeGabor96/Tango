/* date = October 13th 2022 7:29 pm */

#ifndef SIMULATOR_H
#define SIMULATOR_H


#include "common.h"
#include "utils/thread_pool.h"
#include "containers/array.h"
#include "simulator/network.h"
#include "simulator/data_gen.h"
#include "simulator/callback.h"


#define SIMULATOR_N_MAX_CALLBACKS 5u

typedef struct Simulator {
    ThreadPool* pool;
    Network* network;
    DataGen* data;
    Callback* callbacks[SIMULATOR_N_MAX_CALLBACKS];
    u32 n_callbacks;
} Simulator;


internal Simulator* simulator_create(State* state, ThreadPool* pool,
                                     Network* network, DataGen* data);

internal void simulator_run(State* state, Simulator* simulator);
internal void simulator_add_callback(State* state, Simulator* simulator, Callback* callback);
internal void simulator_destroy(Simulator* simulator);

#endif //SIMULATOR_H
