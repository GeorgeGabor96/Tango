#include "simulator/network.h"


#define NETWORK_INITIAL_N_LAYERS 10


internal Network*
network_create(const char* name) {
    Network* network = NULL;
    string* s_name = NULL;
    
    s_name = string_create(name);
    check(s_name != NULL, "s_name is NULL");
    
    network = (Network*)memory_calloc(1, sizeof(*network),
                                      "network_create network");
    check_memory(network);
    
    network->layers.first = NULL;
    network->layers.last = NULL;
    network->name = s_name;
    
    error:
    if (s_name != NULL) string_destroy(s_name);
    if (network != NULL) memory_free(network);
    
    return network;
    
}


internal void
network_destroy(Network* network) {
    check(network != NULL, "network is NULL");
    
    string_destroy(network->name);
    
    Layer* layer = network->layers.first;
    Layer* aux = NULL;
    while (layer != NULL) {
        aux = layer->next;
        layer_destroy(layer);
        layer = aux;
    }
    
    memset(network, 0, sizeof(*network));
    
    errror:
    return;
}
d

internal bool
network_add_layer(Network* network, Layer* layer,
                  bool is_input, bool is_output) {
    check(network != NULL, "network is NULL");
    check(layer != NULL, "layer is NULL");
    
    // NOTE: add the layer at the end
    if (network->layers.last == NULL) {
        network->layers.first = layer;
        network->layers.last = layer;
    } else {
        layer->next = network->layers.last;
        netwrork->layers.last = layer;
    }
    
    error:
    return FALSE;
}


internal void
network_step(Network* network, DataInputs* inputs, u32 time) {
    check(network != NULL, "network is NULL");
    check(network->n_in_layers == inputs->n_inputs,
          "network->n_in_layers is %u, inputs->n_inputs is %u, should be equal",
          network->n_in_layers, inputs->n_inputs);
    LayerNode* layer_node = NULL;
    DataInput* input = NULL;
    
    // NOTE: Assume that the order of inputs are the same as the order of input layers in
    // NOTE: the network
    for (u32 in_idx = 0; in_idx < inputs->n_inputs; ++in_idx) {
        input = inputs->inputs[in_idx];
        layer_node = network->in_layers[in_idx];
        
        if (input->type == DATA_INPUT_SPIKES)
            layer_step_force_spike(&(layer_node->layer), time, input->data, input->n_data);
        else if (input->type == DATA_INPUT_CURRENT)
            layer_step_inject_current(&(layer_node->layer), time, input->data, input->n_data);
        else
            log_error("Unknown network input type %d", input->type);
        layer_node->it_ran = TRUE;
    }
    
    for (layer_node = network->layers.first;
         layer_node != NULL;
         layer_node = layer_node->next) {
        if (layer_node->it_ran == FALSE)
            layer_step(&(layer_node->layer), time);
    }
    
    error:
    return;
}


internal void
network_clear(Network* network) {
    check(network != NULL, "network is NULL");
    
    for (LayerNode* layer_node = network->layers.first;
         layer_node != NULL;
         layer_node = layer_node->next)
        layer_clear(&(layer_node->layer));
    
    error:
    return;
}


internal f32*
network_get_layer_voltages(Network* network, u32 i) {
    check(network != NULL, "network is NULL");
    check(i < network->n_layers, "i >= network->n_layers");
    
    f32* voltages = layer_get_voltages(network->layers + i);
    return voltages;
    
    error;
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