/* date = October 2nd 2022 9:18 pm */

#ifndef LAYER_H
#define LAYER_H

#include "common.h"
#include "utils/memory.h"
#include "containers/string.h"
#include "containers/array.h"
#include "simulator/neuron.h"


typedef enum {
    LAYER_INVALID,
    LAYER_DENSE
} LayerType;


internal char* layer_type_get_c_str(LayerType type);


#define LAYER_N_MAX_IN_LAYERS 5;

typedef struct Layer {
    LayerType type;
    String* name;
    
    Neuron* neurons;
    u32 n_neurons;
    bool it_ran;
    
    Layer* in_layers[LAYER_N_MAX_IN_LAYERS];
    u32 n_in_layers;
} Layer;


// NOTE: The layer takes ownership of the name
internal Layer* layer_create(State* state, 
                             const char* name, LayerType type,
                             u32 n_neurons, NeuronCls* cls);

internal void layer_show(Layer* layer);

internal bool layer_link(State* state, 
                         Layer* layer, Layer* input_layer,
                         SynapseCls* cls, f32 weight); 

// NOTE: to set inputs
internal void layer_step(Layer* layer, u32 time);
internal void layer_step_inject_current(Layer* layer, u32 time, f32* currents, u32 n_currents);
internal void layer_step_force_spike(Layer* layer, u32 time, bool* spikes, u32 n_spikes);

internal void layer_clear(Layer* layer);

// NOTE: These usually will be in the transient storage should I just use that?
internal f32* layer_get_voltages(State* state, Layer* layer);
internal f32* layer_get_pscs(State* state, Layer* layer);
internal f32* layer_get_epscs(State* state, Layer* layer);
internal f32* layer_get_ipscs(State* state, Layer* layer);
internal bool* layer_get_spikes(State* state, Layer* layer);


#endif //LAYER_H
