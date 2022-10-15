/* date = October 13th 2022 7:29 pm */

#ifndef SIMULATOR_H
#define SIMULATOR_H


#include "common.h"
#include "simulator/network.h"
#include "simulator/data_gen.h"
#include "simulator/callback.h"


typedef struct Simulator {
    Network* network;
    Data* data;
    Callback* callbacks;
} Simulator;


internal Simulator* simulator_create(Network* network,
                                     DataGenerator* data,
                                     Callback* callback);
internal void simulator_destroy(Simulator* simulator);

internal void simulator_run(Simulator* sim);


#endif //SIMULATOR_H
