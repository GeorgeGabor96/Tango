/* date = October 13th 2022 7:29 pm */

#ifndef SIMULATOR_H
#define SIMULATOR_H


#include "common.h"
#include "utils/thread_pool.h"
#include "containers/array.h"
#include "simulator/network.h"
#include "simulator/data_gen.h"
#include "simulator/callback.h"


typedef enum {
    SIMULATOR_INFER,
    SIMULATOR_LEARNING,
    SIMULATOR_INVALID_MODE
} SimulatorMode;
// TODO: we could use function pointers to change behaviour based on mode and maybe have a simulator_set_mode() that will change them for us
// NOTE: for now set the mode by hand

internal const char*
simulator_mode_get_c_str(SimulatorMode mode) {
    if (mode == SIMULATOR_INFER) return "SIMULATOR_INFER";
    else if (mode == SIMULATOR_LEARNING) return "SIMULATOR_LEARNING";
    else return "SIMULATOR_INVALID_MODE";
}


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

#endif //SIMULATOR_H
