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

internal void simulator_infer(Simulator* sim) {
    check(simulator != NULL, "simulator is NULL");
    DataSample* sample = NULL;
    u32 data_idx = 0;
    u32 time = 0;
    Callback* callback = NULL;
    
    for (data_idx = 0; data_idx < simulator->data; ++data_idx) {
        sample = data_gen_get_sample(simulator->data, data_idx);
        
        // TODO: clear the network
        for (time = 0; time < sample->duration; ++time) {
            data = data_sample_get_at_time(sample, time);
            
            network_step(sim->network);
            
            callback = sim->callbacks;
            while (callback) {
                callback_update(callback, sim->network);
                callback = callback->next;
            }
            
            // TODO: free the data
        }
        
        callback = sim->callbacks;
        while (callback) {
            callback_run(callback, sim->network);
            callback = callback->next;
        }
    }
    
    error:
    return;
}



#endif //SIMULATOR_H
