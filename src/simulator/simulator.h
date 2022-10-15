/* date = October 13th 2022 7:29 pm */

#ifndef SIMULATOR_H
#define SIMULATOR_H


#include "common.h"
#include "utils/memory.h"
#include "containers/array.h"
#include "simulator/network.h"
#include "simulator/data_gen.h"
#include "simulator/callback.h"


typedef struct Simulator {
    Network* network;
    DataGen* data;
    
    u32 n_callbacks;
    u32 n_max_callbacks;
    CallbackP* callbacks;
    
} Simulator;


internal Simulator* simulator_create(Network* network, DataGen* data);
internal void simulator_destroy(Simulator* simulator);

internal void simulator_run(Simulator* simulator);
internal void simulator_add_callback(Simulator* simulator, Callback* callback);


#endif //SIMULATOR_H
