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
    
    network->layers = (Layers*)memory_calloc(NETWORK_INITIAL_N_LAYERES,
                                             sizeof(Layer),
                                             "network_create layers");
    check_memory(network->layers);
    network->n_layers = 0;
    network->n_max_layers = NETWORK_INITIAL_N_LAYERS;
    
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
    memory_free(network->layers);
    memset(network, 0, sizeof(*network));
    
    errror:
    return;
}


internal bool
network_add_layer(Network* network, Layer* layer,
                  bool is_input, bool is_output) {
    check(network != NULL, "network is NULL");
    check(layer != NULL, "layer is NULL");
    
    // NOTE: resize layers array if needed
    if (network->n_layers == network->n_max_layers) {
        // TODO: an array resize function like a general one
        // TODO: like array_resize(void* array, el_size, current_len, new_len)
        // TODO: and be this the general part not the full array?
        u32 new_n_max_layers = network->n_layers + 10;
        Lyaer* new_layers = (Layers*) array_increase_capacty(network->layers,
                                                             sizeof(Layer),
                                                             network->n_layers,
                                                             new_n_max_layers);
        check(new_layers != NULL, "new_layers is NULL");
        layer->layers = new_layers;
        layer->n_max_layers = new_n_max_layers;
    }
    
    memcpy(layer->layers + network->n_layers, layer, sizeof(*layer));
    memset(layer, 0, sizeof(*layer));
    memory_free(layer);
    
    error:
    return FALSE;
}


internal void
network_step(Network* network, u32 time) {
    check(network != NULL, "network is NULL");
    
    for (u32 i = 0; i < network->n_layers; ++i)
        layer_step(network->layers + i, time);
    
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