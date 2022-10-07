/* date = October 2nd 2022 9:18 pm */

#ifndef LAYER_H
#define LAYER_H

#include "common.h"
#include "containers/string.h"
#include "containers/array.h"
#include "simulator/neuron.h"


typedef enum {
    LAYER_INVALID,
    LAYER_DENSE
} LayerType;


internal char* layer_type_get_c_str(LayerType type);


typedef struct Layer {
    String* name;
    
    Neurons* neurons;
    u32 n_neurons;
    
    LayerType type;
} Layer;


// NOTE: The layer takes ownership of the name
internal Layer* layer_create(String* name, LayerType type,
                             u32 n_neurons, NeuronCls* cls);
internal bool layer_init(Layer* layer, String* name, LayerType type,
                         u32 n_neurons, NeuronCls* cls);
internal void layer_destroy(Layer* layer);
internal void layer_reset(Layer* layer);

internal void layer_step(Layer* layer, u32 time);

internal void layer_show(Layer* layer);

internal bool layer_link(Layer* layer, Layer* input_layer); 


#endif //LAYER_H
