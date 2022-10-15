/* date = October 7th 2022 10:32 pm */

#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"
#include "utils/memory.h"
#include "containers/string.h"
#include "simulator/layer.h"


typedef struct Network {
    String* name;
    
    u32 n_layers;
    u32 n_max_layers;
    Layer* layers;
    
    u32 n_in_layers;
    u32 n_max_in_layers;
    u32* in_layers_idxs;
    
    u32 n_out_layers;
    u32 n_max_out_layers;
    u32* out_layers_idxs;
    
    // TODO: Do i need these??
    SynapseCls* synapse_classes;
    NeuronCls* neuron_classes;
} Network;


/************************
* INPUTS FOR THE NETWORK
************************/
typedef enum {
    NETWORK_INPUT_INVALID,
    NETWORK_INPUT_SPIKES,
    NETWORK_INPUT_CURRENT,
} NetworkInputType;


typedef struct NetworkInput {
    NetworkInputType type;
    void* data;
    u32 n_values;
} NetworkInput;


typedef struct NetworkInputs {
    NetworkInput* inputs;
    u32 n_inputs;
} NetworkInputs;


internal Network* network_create(const char* name);
internal void network_destroy(Network* network);

internal void network_show(Network* network);
internal void network_compile(Network* network);

// NOTE: the layer is moved into the network, it takes ownership, don't use it
// NOTE: after this
internal bool network_add_layer(Network* network, Layer* layer,
                                bool is_input, bool is_output);
internal void network_step(Network* network, NetworkInputs* inputs, u32 time);
internal void network_clear(Network* network);

internal f32* network_get_layer_voltages(Network* network, u32 i);
internal bool* network_get_layer_spikes(Network* network, u32 i);
// TODO: probably need some for psc

internal void network_inputs_destroy(NetworkInputs* inputs);

#endif //NETWORK_H
