#include "simulator/network.h"


#define NETWORK_INITIAL_N_LAYERS 10


internal Network*
network_create(const char* name) {
    Network* network = NULL;
    
    network = (Network*)memory_calloc(1, sizeof(*network),
                                      "network_create network");
    check_memory(network);
    
    network->name = string_create(name);
    check(network->name != NULL, "network->name is NULL");
    
    network->n_layers = 0;
    network->n_max_layers = NETWORK_INITIAL_N_LAYERS; 
    network->layers = (Layer*)memory_malloc(sizeof(Layer) * network->n_layers,
                                            "network_create network->layers");
    
    network->n_in_layers = 0;
    network->n_max_in_layers = 1; 
    network->in_layers_idxs = (u32*)memory_malloc(sizeof(u32) * network->n_max_in_layers,
                                                  "network_create network->in_layers_idxs");
    
    network->n_out_layers = 0;
    network->n_max_out_layers = 1; 
    network->out_layers_idxs = (u32*)memory_malloc(sizeof(u32) * network->n_max_out_layers,
                                                   "network_create network->out_layers_idxs");
    
    error:
    if (network != NULL) {
        if (network->name) string_destroy(network->name);
        if (network->layers) memory_free(network->layers);
        if (network->in_layers_idxs) memory_free(network->in_layers_idxs);
        if (network->out_layers_idxs) memory_free(network->out_layers_idxs);
        memory_free(network);
    }
    
    return network;
    
}


internal void
network_destroy(Network* network) {
    check(network != NULL, "network is NULL");
    
    string_destroy(network->name);
    memory_free(network->layers);
    memory_free(network->in_layers_idxs);
    memory_free(network->out_layers_idxs);
    
    memset(network, 0, sizeof(*network));
    memory_free(network);
    
    error:
    return;
}


internal bool
network_add_layer(Network* network, Layer* layer,
                  bool is_input, bool is_output) {
    check(network != NULL, "network is NULL");
    check(layer != NULL, "layer is NULL");
    
    if (network->n_layers == network->n_max_layers) {
        u32 new_n_max_layers = network->n_max_layers * 2;
        network->layers = array_resize(network->layers, sizeof(Layer),
                                       network->n_max_layers, new_n_max_layers);
        check(network->layers != NULL, "network->layers is NULL");
        network->n_max_layers = new_n_max_layers;
    }
    memcpy(network->layers + network->n_layers, layer, sizeof(*layer));
    memory_free(layer);
    
    if (is_input == TRUE) {
        if (network->n_in_layers == network->n_max_in_layers) {
            u32 new_n_max_in_layers = network->n_max_in_layers * 2;
            network->in_layers_idxs = array_resize(network->in_layers_idxs, sizeof(u32),
                                                   network->n_max_in_layers,
                                                   new_n_max_in_layers);
            check(network->in_layers_idxs != NULL, "network->in_layers_idxs is NULL");
            network->n_max_in_layers = new_n_max_in_layers;
        }
        network->in_layers_idxs[network->n_in_layers] = network->n_layers;
        ++(network->n_in_layers);
    }
    
    if (is_output == TRUE) {
        if (network->n_out_layers == network->n_max_out_layers) {
            u32 new_n_max_out_layers = network->n_max_out_layers * 2;
            network->out_layers_idxs = array_resize(network->out_layers_idxs, sizeof(u32),
                                                    network->n_max_out_layers,
                                                    new_n_max_out_layers);
            check(network->out_layers_idxs != NULL, "network->out_layers_idxs is NULL");
            network->n_max_out_layers = new_n_max_out_layers;
        }
        network->out_layers_idxs[network->n_out_layers] = network->n_layers;
        ++(network->n_out_layers);
    }
    
    ++(network->n_layers);
    
    error:
    return FALSE;
}


internal void
network_step(Network* network, NetworkInputs* inputs, u32 time) {
    check(network != NULL, "network is NULL");
    check(network->n_in_layers == inputs->n_inputs,
          "network->n_in_layers is %u, inputs->n_inputs is %u, should be equal",
          network->n_in_layers, inputs->n_inputs);
    Layer* layer = NULL;
    NetworkInput* input = NULL;
    u32 i = 0;
    
    // NOTE: Assume that the order of inputs are the same as the order of input layers in
    // NOTE: the network
    for (i = 0; i < inputs->n_inputs; ++i) {
        input = inputs->inputs + i;
        layer = network->layers + network->in_layers_idxs[i];
        
        if (input->type == NETWORK_INPUT_SPIKES)
            layer_step_force_spike(layer, time, input->data, input->n_values);
        else if (input->type == NETWORK_INPUT_CURRENT)
            layer_step_inject_current(layer, time, input->data, input->n_values);
        else
            log_error("Unknown network input type %d", input->type);
        layer->it_ran = TRUE;
    }
    
    for (i = 0; i < network->n_layers; ++i) {
        layer = network->layers + i;
        if (layer->it_ran == FALSE)
            layer_step(layer, time);
    }
    
    error:
    return;
}


internal void
network_clear(Network* network) {
    check(network != NULL, "network is NULL");
    
    for (u32 i = 0; i < network->n_layers; ++i)
        layer_clear(network->layers + i);
    
    error:
    return;
}


internal f32*
network_get_layer_voltages(Network* network, u32 i) {
    check(network != NULL, "network is NULL");
    check(i < network->n_layers, "i >= network->n_layers");
    
    f32* voltages = layer_get_voltages(network->layers + i);
    return voltages;
    
    error:
    return NULL;
}


internal bool*
network_get_layer_spikes(Network* network, u32 i) {
    check(network != NULL, "network is NULL");
    check(i < network->n_layers, "i >= network->n_layers");
    
    bool* spikes = layer_get_spikes(network->layers + i);
    return spikes;
    
    error:
    return NULL;
}


internal void
network_inputs_destroy(NetworkInputs* inputs) {
    check(inputs != NULL, "inputs is NULL");
    NetworkInput* input = NULL;
    u32 i = 0;
    
    for (i = 0; i < inputs->n_inputs; ++i) {
        input = inputs->inputs + i;
        memory_free(input->data);
        memset(input, 0, sizeof(*input));
    }
    memset(inputs, 0, sizeof(*inputs));
    memory_free(inputs);
    
    error:
    return;
}