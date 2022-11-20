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
    simulator->n_callbacks = 0;
    
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
    u32 data_idx = 0;
    u32 time = 0;
    u32 callback_i = 0;
    
    // TIMING
    clock_t total_time_start = 0;
    clock_t total_time = 0;
    
    clock_t sample_time_start = 0;
    clock_t sample_time = 0;
    f64 sample_time_s = 0.0;
    
    clock_t network_time_start = 0;
    clock_t network_time = 0;
    f64 network_time_s = 0.0;
    
    random_init();
    
    total_time_start = clock();
    for (data_idx = 0; data_idx < simulator->data->length; ++data_idx) {
        sample_time_start = clock();
        
        sample = data_gen_sample_create(state->transient_storage,
                                        simulator->data, data_idx);
        
        network_clear(simulator->network);
        
        for (callback_i = 0; callback_i < simulator->n_callbacks; ++callback_i)
            callback_begin_sample(state,
                                  simulator->callbacks[callback_i],
                                  simulator->network,
                                  sample->duration);
        
        for (time = 0; time < sample->duration; ++time) {
            inputs = data_network_inputs_create(state->transient_storage,
                                                sample,
                                                simulator->network,
                                                time);
            
            network_time_start = clock();
            network_step(simulator->network, inputs, time);
            network_time += clock() - network_time_start;
            
            for (callback_i = 0;
                 callback_i < simulator->n_callbacks;
                 ++callback_i)
                callback_update(state,
                                simulator->callbacks[callback_i],
                                simulator->network);
        }
        
        for (callback_i = 0; callback_i < simulator->n_callbacks; ++callback_i)
            callback_end_sample(state,
                                simulator->callbacks[callback_i],
                                simulator->network);
        
        memory_arena_clear(state->transient_storage);
        
        sample_time = clock() - sample_time_start;
        
        // NOTE: Timing logging
        network_time_s = (f64)network_time / CLOCKS_PER_SEC;
        printf("\n");
        log_info("Network time %lfs", network_time_s);
        log_info("Network step time %lfs",
                 network_time_s / (f64)sample->duration); 
        network_time = 0;
        
        sample_time_s = (f64)sample_time / CLOCKS_PER_SEC;
        log_info("Sample time %lfs", sample_time_s);
        log_info("Non NETWORK time %lfs", sample_time_s - network_time_s);
        sample_time = 0;
        
        printf("\n");
    }
    total_time = clock() - total_time_start;
    log_info("Total simulation time %lfs\n", (f64)total_time / CLOCKS_PER_SEC);
    
    TIMING_COUNTER_END(SIMULATOR_RUN);
    
    error:
    return;
}


internal void
simulator_add_callback(State* state, Simulator* simulator, Callback* callback) {
    check(state != NULL, "state is NULL");
    check(simulator != NULL, "simulator is NULL");
    check(callback != NULL, "callback is NULL");
    check(simulator->n_callbacks <= SIMULATOR_N_MAX_CALLBACKS,
          "Cannot add more callbacks: simulator->n_callbacks %u, SIMULATOR_N_MAX_CALLBACKS %u",
          simulator->n_callbacks, SIMULATOR_N_MAX_CALLBACKS);
    
    simulator->callbacks[simulator->n_callbacks] = callback;
    ++(simulator->n_callbacks);
    
    error:
    return;
}