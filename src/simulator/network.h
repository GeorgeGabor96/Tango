/* date = October 7th 2022 10:32 pm */

#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"
#include "utils/memory.h"
#include "containers/string.h"
#include "simulator/layer.h"


#define NETWORK_N_MAX_LAYERS 32

typedef struct Network {
    String* name;
    
    Layer* layers[NETWORK_N_MAX_LAYERS];
    Layer* in_layers[NETWORK_N_MAX_LAYERS];
    Layer* out_layers[NETWORK_N_MAX_LAYERS];
    
    u32 n_layers;
    u32 n_in_layers;
    u32 n_out_layers;
    
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
    u32 n_neurons;
} NetworkInput;


typedef struct NetworkInputs {
    NetworkInput* inputs;
    u32 n_inputs;
} NetworkInputs;


internal Network* network_create(State* state, const char* name);

internal void network_show(Network* network);
internal void network_compile(Network* network);

internal void network_add_layer(State* state,
                                Network* network, Layer* layer,
                                bool is_input, bool is_output);
internal void network_step(Network* network, NetworkInputs* inputs, u32 time);
internal void network_clear(Network* network);

internal f32* network_get_layer_voltages(State* state, Network* network, u32 i);
internal bool* network_get_layer_spikes(State* state, Network* network, u32 i);


#endif //NETWORK_H
