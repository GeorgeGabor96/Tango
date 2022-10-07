/* date = October 7th 2022 10:32 pm */

#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"
#include "containers/string.h"
#include "simulator/layer.h"


typedef struct Network {
    String* name;
    Layer* layers;
    
    // NOTE: much easier to implement
    // NOTE: assume a network has at most 10 inputs or outputs
    u32 in_layers_idxs[10];
    u32 out_layers_idxs[10];
    
    u32 n_layers;
    u32 n_max_layers;
    u32 n_in_layers;
    u32 n_out_layers;
    
} Network;


internal Network* network_create(const char* name);
internal Network* network_create(String* name);
internal void network_destroy(Network* network);

// NOTE: the layer is moved into the network, it takes ownership, don't use it
// NOTE: after this
internal bool network_add_layer(Network* network, Layer* layer,
                                bool is_input, bool is_output);


#endif //NETWORK_H
