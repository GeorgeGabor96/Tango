#include "simulator/network.h"


internal Simulator*
simulator_create(Network* network, DataGenerator* data, Callback* callbacks) {
    
    
}

internal void
simulator_run(Simulator* sim)
{
    check(simulator != NULL, "simulator is NULL");
    DataSample* sample = NULL;
    DataInputs* inputs = NULL;
    Callback* callback = NULL;
    u32 data_idx = 0;
    u32 time = 0;
    
    for (data_idx = 0; data_idx < simulator->data; ++data_idx) {
        sample = data_gen_sample_create(simulator->data, data_idx);
        
        network_clear(simulator->network);
        
        for (time = 0; time < sample->duration; ++time) {
            inputs = data_gen_inputs_create(sample, network, time);
            
            network_step(sim->network, inputs, time);
            
            callback = sim->callbacks;
            while (callback) {
                callback_update(callback, sim->network);
                callback = callback->next;
            }
            
            data_inputs_destroy(inputs);
        }
        
        data_gen_sample_destroy(sample);
        
        callback = sim->callbacks;
        while (callback) {
            callback_run(callback, sim->network);
            callback = callback->next;
        }
    }
    
    error:
    return;
}
