/* date = October 13th 2022 7:29 pm */

#ifndef SIMULATOR_H
#define SIMULATOR_H


#include "common.h"
#include "simulator/network.h"


typedef struct Simulator {
    Network* network;
    DataGenerator* data;
    Callback* callbacks;
} Simulator;


internal Simulator* simulator_create(Network* network,
                                     DataGenerator* data,
                                     Callback* callback);
void simulator_destroy(Simulator* simulator);



#endif //SIMULATOR_H
