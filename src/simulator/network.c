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
    network->layers =
    (Layer*)memory_malloc(sizeof(Layer) * network->n_max_layers,
                          "network_create network->layers");
    check_memory(network->layers);
    
    network->n_in_layers = 0;
    network->n_max_in_layers = 1; 
    network->in_layers_idxs = (u32*)memory_malloc(sizeof(u32) * network->n_max_in_layers,
                                                  "network_create network->in_layers_idxs");
    check_memory(network->in_layers_idxs);
    
    network->n_out_layers = 0;
    network->n_max_out_layers = 1; 
    network->out_layers_idxs = (u32*)memory_malloc(sizeof(u32) * network->n_max_out_layers,
                                                   "network_create network->out_layers_idxs");
    check_memory(network->out_layers_idxs);
    
    network->n_synapse_clss = 0;
    network->n_max_synapse_clss = 5;
    network->synapse_clss = (SynapseCls*) memory_malloc(sizeof(SynapseCls) * network->n_max_synapse_clss,
                                                        "network_create network->synapse_clss");
    check_memory(network->synapse_clss);
    
    network->n_neuron_clss = 0;
    network->n_max_neuron_clss = 5;
    network->neuron_clss = (NeuronCls*) memory_malloc(sizeof(NeuronCls) * network->n_max_neuron_clss,
                                                      "network_create network->neuron_clss");
    check_memory(network->neuron_clss);
    
    return network;
    
    error:
    if (network != NULL) {
        if (network->name) string_destroy(network->name);
        if (network->layers) memory_free(network->layers);
        if (network->in_layers_idxs) memory_free(network->in_layers_idxs);
        if (network->out_layers_idxs) memory_free(network->out_layers_idxs);
        if (network->synapse_clss) memory_free(network->synapse_clss);
        if (network->neuron_clss) memory_free(network->neuron_clss);
        memory_free(network);
    }
    
    return NULL;
}


internal void
network_destroy(Network* network) {
    check(network != NULL, "network is NULL");
    u32 i = 0;
    
    string_destroy(network->name);
    
    for (i = 0; i < network->n_layers; ++i)
        layer_reset(network->layers + i);
    memory_free(network->layers);
    
    memory_free(network->in_layers_idxs);
    memory_free(network->out_layers_idxs);
    
    // NOTE: Network own the synapse and neuron classes
    for (i = 0; i < network->n_synapse_clss; ++i)
        synapse_cls_reset(network->synapse_clss + i);
    memory_free(network->synapse_clss);
    
    for (i = 0; i < network->n_neuron_clss; ++i)
        neuron_cls_reset(network->neuron_clss + i);
    memory_free(network->neuron_clss);
    
    memset(network, 0, sizeof(*network));
    memory_free(network);
    
    error:
    return;
}


internal void
network_show(Network* network) {
    check(network != NULL, "network is NULL");
    u32 i = 0;
    Layer* layer = NULL;
    
    printf("-----------------------NETWORK---------------------\n");
    printf("Name: %s\n\n", string_to_c_str(network->name));
    
    printf("Layers:\n");
    for (i = 0; i < network->n_layers; ++i)  
        layer_show(network->layers + i);
    printf("Number of layers: %u\n\n", network->n_layers);
    
    printf("Input Layers: ");
    for (i = 0; i < network->n_in_layers; ++i) {
        layer = network->layers + network->in_layers_idxs[i];
        printf("%s, ", string_to_c_str(layer->name));
    }
    printf("\nNumber of input layers: %u\n\n", network->n_in_layers);
    
    printf("Output Layers: ");
    for (i = 0; i < network->n_out_layers; ++i) {
        layer = network->layers + network->out_layers_idxs[i];
        printf("%s, ", string_to_c_str(layer->name));
    }
    printf("\nNumber of output layers: %u\n\n", network->n_out_layers);
    // TODO: add number fo parameters
    printf("-----------------------NETWORK---------------------\n");
    
    
    error:
    return;
}


internal SynapseCls*
network_add_synapse_cls(Network* network, SynapseCls* cls) {
    check(network != NULL, "network is NULL");
    check(cls != NULL, "cls is NULL");
    
    if (network->n_synapse_clss == network->n_max_synapse_clss) {
        u32 new_n_max_synapse_clss = network->n_max_synapse_clss * 2;
        network->synapse_clss = array_resize(network->synapse_clss, sizeof(*cls),
                                             network->n_max_synapse_clss,
                                             new_n_max_synapse_clss);
        check(network->synapse_clss != NULL, "network->synapse_clss is NULL");
        network->n_max_synapse_clss = new_n_max_synapse_clss;
    }
    SynapseCls* new_cls = network->synapse_clss + network->n_synapse_clss;
    memcpy(new_cls, cls, sizeof(*cls));
    memset(cls, 0, sizeof(*cls));
    memory_free(cls);
    ++(network->n_synapse_clss);
    return new_cls;
    
    error:
    return NULL;
}


internal NeuronCls*
network_add_neuron_cls(Network* network, NeuronCls* cls) {
    check(network != NULL, "network is NULL");
    check(cls != NULL, "cls is NULL");
    
    if (network->n_neuron_clss == network->n_max_neuron_clss) {
        u32 new_n_max_neuron_clss = network->n_max_neuron_clss * 2;
        network->neuron_clss = array_resize(network->neuron_clss, sizeof(*cls),
                                            network->n_max_neuron_clss,
                                            new_n_max_neuron_clss);
        check(network->neuron_clss != NULL, "network->neuron_clss is NULL");
        network->n_max_neuron_clss = new_n_max_neuron_clss;
    }
    NeuronCls* new_cls = network->neuron_clss + network->n_neuron_clss;
    memcpy(new_cls, cls, sizeof(*cls));
    memset(cls, 0, sizeof(*cls));
    memory_free(cls);
    ++(network->n_neuron_clss);
    return new_cls;
    
    error:
    return NULL;
}


internal bool
network_add_layer(Network* network, Layer* layer,
                  bool is_input, bool is_output) {
    check(network != NULL, "network is NULL");
    check(layer != NULL, "layer is NULL");
    
    printf("%u %u\n", network->n_layers, network->n_max_layers);
    if (network->n_layers == network->n_max_layers) {
        u32 new_n_max_layers = network->n_max_layers * 2;
        network->layers = array_resize(network->layers, sizeof(Layer),
                                       network->n_max_layers, new_n_max_layers);
        check(network->layers != NULL, "network->layers is NULL");
        network->n_max_layers = new_n_max_layers;
    }
    memcpy(network->layers + network->n_layers, layer, sizeof(*layer));
    memset(layer, 0, sizeof(*layer));
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
    memory_free(inputs->inputs);
    memset(inputs, 0, sizeof(*inputs));
    memory_free(inputs);
    
    error:
    return;
}