#include "simulator/network.h"


internal Network*
network_create(State* state, const char* name) {
    check(state != NULL, "state is NULL");
    check(name != NULL, "name is NULL");
    
    Network* network = NULL;
    
    network = (Network*)memory_arena_push(state->permanent_storage, sizeof(*network));
    check_memory(network);
    
    network->name = string_create(state->permanent_storage, name);
    check(network->name != NULL, "network->name is NULL");
    
    memset(network->layers, 0, sizeof(Layer*) * NETWORK_N_MAX_LAYERS);
    memset(network->in_layers, 0, sizeof(Layer*) * NETWORK_N_MAX_LAYERS);
    memset(network->out_layers, 0, sizeof(Layer*) * NETWORK_N_MAX_LAYERS);
    
    network->n_layers = 0;
    network->n_in_layers = 0;
    network->n_out_layers = 0;
    
    return network;
    
    error:
    return NULL;
}


internal void
network_show(Network* network) {
    check(network != NULL, "network is NULL");
    u32 i = 0;
    u32 n_synapses = 0;
    u32 n_neurons = 0;
    Layer* layer = NULL;
    
    printf("-----------------------NETWORK---------------------\n");
    printf("Name: %s\n\n", string_to_c_str(network->name));
    
    printf("Layers:\n");
    for (i = 0; i < network->n_layers; ++i) {
        layer = network->layers[i];
        layer_show(layer);
        n_synapses += layer_get_n_in_synapses(layer);
        n_neurons += layer->n_neurons;
    }
    printf("Number of layers: %u\n\n", network->n_layers);
    
    printf("Input Layers: ");
    for (i = 0; i < network->n_in_layers; ++i)
        printf("%s, ", string_to_c_str(network->layers[i]->name));
    printf("\nNumber of input layers: %u\n\n", network->n_in_layers);
    
    printf("Output Layers: ");
    for (i = 0; i < network->n_out_layers; ++i)
        printf("%s, ", string_to_c_str(network->layers[i]->name));
    printf("\nNumber of output layers: %u\n\n", network->n_out_layers);
    
    printf("Number of neurons: %u\n", n_neurons);
    printf("Number of synapses: %u\n", n_synapses);
    printf("-----------------------NETWORK---------------------\n");
    
    error:
    return;
}


internal void
network_add_layer(Network* network, Layer* layer,
                  bool is_input, bool is_output) {
    check(network != NULL, "network is NULL");
    check(layer != NULL, "layer is NULL");
    
    check(network->n_layers < NETWORK_N_MAX_LAYERS,
          "network->n_layers %u  >= NETWORK_N_MAX_LAYERS %u, increase array size",
          network->n_layers, NETWORK_N_MAX_LAYERS);
    
    network->layers[network->n_layers] = layer;
    ++(network->n_layers);
    
    if (is_input == TRUE) {
        network->in_layers[network->n_in_layers] = layer;
        ++(network->n_in_layers);
    }
    
    if (is_output == TRUE) {
        network->out_layers[network->n_out_layers] = layer;
        ++(network->n_out_layers);
    }
    
    error:
    return;
}


internal void
network_step(Network* network, NetworkInputs* inputs, u32 time,
             MemoryArena* storage, ThreadPool* pool) {
    TIMING_COUNTER_START(NETWORK_STEP);
    
    check(network != NULL, "network is NULL");
    check(network->n_in_layers == inputs->n_inputs,
          "network->n_in_layers is %u, inputs->n_inputs is %u, should be equal",
          network->n_in_layers, inputs->n_inputs);
    check(storage != NULL, "storage is NULL");
    check(pool != NULL, "pool is NULL");
    
    Layer* layer = NULL;
    NetworkInput* input = NULL;
    u32 i = 0;
    
    // NOTE: Assume that the order of inputs are the same as the order of input layers in
    // NOTE: the network
    for (i = 0; i < inputs->n_inputs; ++i) {
        input = inputs->inputs + i;
        layer = network->layers[i];
        
        if (input->type == NETWORK_INPUT_SPIKES)
            layer_step_force_spike(layer, time, input->data, input->n_neurons,
                                   storage, pool);
        else if (input->type == NETWORK_INPUT_CURRENT)
            layer_step_inject_current(layer, time, input->data, input->n_neurons,
                                      storage, pool);
        else
            log_error("Unknown network input type %d", input->type);
        layer->it_ran = TRUE;
    }
    
    for (i = 0; i < network->n_layers; ++i) {
        layer = network->layers[i];
        if (layer->it_ran == FALSE)
            layer_step(layer, time, storage, pool);
    }
    
    TIMING_COUNTER_END(NETWORK_STEP);
    error:
    return;
}


internal void
network_clear(Network* network) {
    check(network != NULL, "network is NULL");
    
    for (u32 i = 0; i < network->n_layers; ++i)
        layer_clear(network->layers[i]);
    
    error:
    return;
}


internal f32*
network_get_layer_voltages(State* state, Network* network, u32 i) {
    check(state != NULL, "state is NULL");
    check(network != NULL, "network is NULL");
    check(i < network->n_layers, "i >= network->n_layers");
    
    f32* voltages = layer_get_voltages(state->transient_storage,
                                       network->layers[i]);
    return voltages;
    
    error:
    return NULL;
}


internal bool*
network_get_layer_spikes(State* state, Network* network, u32 i) {
    check(state != NULL, "state is NULL");
    check(network != NULL, "network is NULL");
    check(i < network->n_layers, "i >= network->n_layers");
    
    bool* spikes = layer_get_spikes(state->transient_storage,
                                    network->layers[i]);
    return spikes;
    
    error:
    return NULL;
}
