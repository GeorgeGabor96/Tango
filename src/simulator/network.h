/* date = October 7th 2022 10:32 pm */

#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"
#include "containers/string.h"
#include "simulator/layer.h"


typedef struct LayerNode {
    Layer layer;
    bool it_ran;
    struct LayerNode* next;
} LayerNode;


typedef struct LayerList {
    LayerNode* first;
    LayerNode* last;
} LayerList;


typedef struct Network {
    String* name;
    
    // NOTE: for now its simpler to just keep a list of layers
    LayerList layers;
    
    // NOTE: much easier to implement
    // NOTE: assume a network has at most 10 inputs or outputs
    LayerNode* in_layers[10];
    LayerNode* out_layers[10];
    u32 n_in_layers;
    u32 n_out_layers;
    
    SynapticCls* synapse_classes;
    NeuronCls* neuron_classes;
} Network;


internal Network* network_create(const char* name);
internal Network* network_create(String* name);
internal void network_destroy(Network* network);

internal void network_show(Network* network);
internal void network_compile(Network* network);

// NOTE: the layer is moved into the network, it takes ownership, don't use it
// NOTE: after this
internal bool network_add_layer(Network* network, Layer* layer,
                                bool is_input, bool is_output);
internal void network_step(Network* network, NetworkIn* inputs, u32 time);
internal void network_clear(Network* network);

internal f32* network_get_layer_voltages(Network* network, u32 i);
internal bool* network_get_layer_spikes(Network* network, u32 i);
// TODO: probably need some for psc

#endif //NETWORK_H
