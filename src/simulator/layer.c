#include "simulator/layer.h"


/******************
*   LAYER TYPE
******************/
internal char*
layer_type_get_c_str(LayerType type) {
    if (type == LAYER_DENSE) return "LAYER_DENSE";
    else return "LAYER_INVALID";
}


/****************
*   LAYER
****************/
internal Layer*
layer_create(String* name, LayerType type, u32 n_neurons, NeuronCls* cls) {
    Layer* layer = NULL;
    bool flag = FALSE;
    
    layer = (Layer*) memory_malloc(sizeof(layer*), "layer_create");
    check_memory(layer);
    
    flag = layer_init(layer, name, type, n_neurons, cls);
    check(flag == TRUE, "couldn't init the layer");
    
    return layer;
    
    error:
    memory_free(layer);
    return NULL;
}


internal bool
layer_init(Layer* layer, String* name, LayerType type, 
           u32 n_neurons, NeuronCls* cls) {
    check(layer != NULL, "layer is NULL");
    check(name != NULL, "name is NULL");
    check(type == LAYER_DENSE, "invalid layer type %s", 
          layer_type_get_c_str(type));
    check(cls != NULL, "cls is NULL");
    
    Array* neurons = NULL;
    Neuron* neuron = NULL;
    u32 i = 0;
    
    neurons = array_create(n_neurons, sizeof(Neuron));
    for (i = 0; i < neurons->lenght; ++i) {
        neuron = *((Neuron**)array_get(neurons, i));
        neuron_init(neuron, cls);
    }
    
    layer->name = name;
    layer->neurons = neurons;
    layer->type = type;
    
    // TODO:
    if (layer->type == LAYER_DENSE) {
        
    }
    
    return TRUE;
    error:
    return FALSE;
}


internal void
layer_reset(Layer* layer) {
    check(layer != NULL, "layer is NULL");
    
    array_destroy(layer->neurons, neuron_reset_double_p);
    memset(layer, 0, sizeof(*layer));
    
    error:
    return;
}


internal void
layer_destroy(Layer* layer) {
    check(layer != NULL, "layer is NULL");
    layer_reset(layer);
    memory_free(layer);
    
    error:
    return;
}


