#include "simulator/simulator.h"


internal Simulator*
simulator_create(State* state, Network* network, DataGen* data) {
    Simulator* simulator = NULL;
    
    check(state != NULL, "state is NULL");
    check(network != NULL, "network is NULL");
    check(data != NULL, "data is NULL");
    
    simulator = (Simulator*) memory_arena_push(state->permanent_storage, sizeof(*simulator));
    check_memory(simulator);
    
    simulator->network = network;
    simulator->data = data;
    simulator->callback_list = NULL;
    
    return simulator;
    
    error:
    return NULL;
}


internal void
simulator_run(State* state, Simulator* simulator)
{
    TIMING_COUNTER_START(SIMULATOR_RUN);
    
    check(state != NULL, "state is NULL");
    check(simulator != NULL, "simulator is NULL");
    
    DataSample* sample = NULL;
    NetworkInputs* inputs = NULL;
    CallbackNode* callback_node = NULL;
    u32 data_idx = 0;
    u32 time = 0;
    u32 i = 0;
    
    
    for (data_idx = 0; data_idx < simulator->data->length; ++data_idx) {
        sample = data_gen_sample_create(state, simulator->data, data_idx);
        
        network_clear(simulator->network);
        
        for (callback_node = simulator->callback_list;
             callback_node != NULL;
             callback_node = callback_node->next)
            callback_begin_sample(state,
                                  callback_node->callback,
                                  simulator->network,
                                  sample->duration);
        
        for (time = 0; time < sample->duration; ++time) {
            inputs = data_network_inputs_create(state, sample, simulator->network, time);
            time += 1;
            network_step(simulator->network, inputs, time);
            
            for (callback_node = simulator->callback_list;
                 callback_node != NULL;
                 callback_node = callback_node->next)
                callback_update(state,
                                callback_node->callback,
                                simulator->network);
        }
        
        for (callback_node = simulator->callback_list;
             callback_node != NULL;
             callback_node = callback_node->next)
            callback_end_sample(state,
                                callback_node->callback,
                                simulator->network);
        
        // TODO: Where to put the transient arena reset
        // HERE?
    }
    
    TIMING_COUNTER_END(SIMULATOR_RUN);
    
    error:
    return;
}


internal void
simulator_add_callback(State* state, Simulator* simulator, Callback* callback) {
    check(state != NULL, "state is NULL");
    check(simulator != NULL, "simulator is NULL");
    check(callback != NULL, "callback is NULL");
    
    CallbackNode* node = (CallbackNode*) memory_arena_push(state->permanent_storage,
                                                           sizeof(*node));
    check_memory(node);
    
    node->callback = callback;
    node->next = simulator->callback_list;
    simulator->callback_list = node;
    
    error:
    return;
}