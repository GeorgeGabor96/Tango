#include "simulator/network.h"


internal Simulator*
simulator_create(Network* network, DataGenerator* data, Callback* callbacks) {
    check(network != NULL, "network is NULL");
    check(data != NULL, "data is NULL");
    check(callbacks != NULL, "callbacks != NULL");
    
    Simulator* simulator = (Simulator*) memory_malloc(sizeof(*simulator));
    check_memory(simulator);
    
    simulator->newtork = network;
    simulator->data = data;
    simulator->callbacks = callbacks;
    return simulator;
    
    error:
    return NULL;
}


internal void
simulator_destroy(Simulator* simulator) {
    check(simulator != NULL, "simulator is NULL");
    
    network_destroy(simulator->network);
    data_gen_destroy(simulator->data);
    callback_list_destroy(simulator->callbacks);
    memset(simulator, 0, sizeof(*simulator));
    memory_free(simulator);
    
    error;
    return;
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
