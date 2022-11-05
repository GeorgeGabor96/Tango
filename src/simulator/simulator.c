#include "simulator/simulator.h"


#define SIMULATOR_INITIAL_NUMBER_OF_CALLBACKS 3


internal Simulator*
simulator_create(Network* network, DataGen* data) {
    Simulator* simulator = NULL;
    check(network != NULL, "network is NULL");
    check(data != NULL, "data is NULL");
    
    simulator = (Simulator*) memory_calloc(1, sizeof(*simulator), "simulator_create");
    check_memory(simulator);
    
    simulator->network = network;
    simulator->data = data;
    
    simulator->n_callbacks = 0;
    simulator->n_max_callbacks = SIMULATOR_INITIAL_NUMBER_OF_CALLBACKS;
    simulator->callbacks = (CallbackP*) memory_malloc(sizeof(CallbackP) *
                                                      simulator->n_max_callbacks,
                                                      "simulator_create callbacks");
    check_memory(simulator->callbacks);
    return simulator;
    
    error:
    if (simulator != NULL) {
        if (simulator->network != NULL) network_destroy(simulator->network);
        if (simulator->data != NULL) data_gen_destroy(simulator->data);
        if (simulator->callbacks != NULL) memory_free(simulator->callbacks);
    }
    
    return NULL;
}


internal void
simulator_destroy(Simulator* simulator) {
    check(simulator != NULL, "simulator is NULL");
    
    network_destroy(simulator->network);
    data_gen_destroy(simulator->data);
    
    for (u32 i = 0; i < simulator->n_callbacks; ++i)
        callback_destroy(simulator->callbacks[i]);
    memory_free(simulator->callbacks);
    
    memset(simulator, 0, sizeof(*simulator));
    memory_free(simulator);
    
    error:
    return;
}


internal void
simulator_run(Simulator* simulator)
{
    TIMING_COUNTER_START(SIMULATOR_RUN);
    
    check(simulator != NULL, "simulator is NULL");
    DataSample* sample = NULL;
    NetworkInputs* inputs = NULL;
    Callback* callback = NULL;
    u32 data_idx = 0;
    u32 time = 0;
    u32 i = 0;
    
    for (data_idx = 0; data_idx < simulator->data->length; ++data_idx) {
        sample = data_gen_sample_create(simulator->data, data_idx);
        
        network_clear(simulator->network);
        
        for (i = 0; i < simulator->n_callbacks; ++i)
            callback_begin_sample(simulator->callbacks[i], simulator->network, sample->duration);
        
        for (time = 0; time < sample->duration; ++time) {
            inputs = data_network_inputs_create(sample, simulator->network, time);
            
            network_step(simulator->network, inputs, time);
            
            for (i = 0; i < simulator->n_callbacks; ++i)
                callback_update(simulator->callbacks[i], simulator->network);
            
            network_inputs_destroy(inputs);
        }
        
        data_gen_sample_destroy(sample);
        
        for (i = 0; i < simulator->n_callbacks; ++i)
            callback_end_sample(simulator->callbacks[i], simulator->network);
    }
    
    TIMING_COUNTER_END(SIMULATOR_RUN);
    
    error:
    return;
}


internal void
simulator_add_callback(Simulator* simulator, Callback* callback) {
    check(simulator != NULL, "simulator is NULL");
    check(callback != NULL, "callback is NULL");
    
    if (simulator->n_callbacks == simulator->n_max_callbacks) {
        u32 new_n_max_callbacks = simulator->n_max_callbacks * 2;
        simulator->callbacks = array_resize(simulator->callbacks,
                                            sizeof(CallbackP),
                                            simulator->n_max_callbacks,
                                            new_n_max_callbacks);
        check(simulator->callbacks, "simulator->callbacks is NULL");
        simulator->n_max_callbacks = new_n_max_callbacks;
    }
    simulator->callbacks[simulator->n_callbacks] = callback;
    ++(simulator->n_callbacks);
    
    error:
    return;
}