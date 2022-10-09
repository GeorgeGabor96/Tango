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
    
    Neurons* neurons = NULL;
    Neuron* neuron = NULL;
    u32 i = 0;
    
    neurons = (Neurons*)memory_malloc(n_neurons * sizeof(Neuron));
    for (i = 0; i < n_neurons; ++i) {
        neuron_init(neurons + i, cls);
    }
    
    layer->name = name;
    layer->neurons = neurons;
    layer->n_neurons = n_neurons
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
    
    // NOTE: reset the neuron memory
    u32 i = 0;
    for (i = 0; i < layer->n_neurons; ++i) {
        neuron_reset(layer->neurons + i);
    }
    memory_free(layer->neurons);
    
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


internal void
layer_step(Layer* layer, u32 time) {
    assert(layer != NULL, "layer is NULL");
    
    for (u32 i = 0; i < layer->n_neurons; ++i) {
        neuron_step(layer->neurons + i, time);
    }
    
    error:
    return;
}


internal void
layer_show(Layer* layer) {
    check(layer != NULL, "layer is NULL");
    
    u32 n_in_synapses = 0;
    for (u32 i = 0; i < layer->n_neurons; ++i) {
        n_in_synapses += layer->neurons[i].in_synapses_ref->length;
    }
    
    printf("Name: %s\n", string_get_c_str(layer->name));
    printf("Type: %s\n", layer_type_get_c_str(layer->type));
    printf("Number of neurons %u of type %s\n",
           layer->n_neurons,
           string_get_c_str(layer->neurons[0].cls->name));
    printf("Number of input synapses %u\n", n_in_synapses);
    
    error:
    return;
}


inline internal bool
layer_link_dense(Layer* layer, Layer* in_layer,
                 SynapseCls* cls, f32 weight) {
    u32 neuron_i = 0;
    u32 in_neuron_i = 0;
    Neuron* neuron = NULL;
    Neuron* in_neuron = NULL;
    Synapse* synapse = NULL;
    bool status = FALSE;
    
    for (neuron_i = 0; neuron_i < layer->n_neurons; ++neuron_i) {
        neuron = layer->neurons + neuron_i;
        
        for (in_neuron_i = 0; in_neuron_i < in_layer->n_neurons; ++in_neuron_i) {
            in_neuron = in_layer->neurons + i;
            
            synapse = synapse_create(cls, weight);
            check_memory(synapse);
            synapse = neuron_add_in_synapse(neuron, synapse);
            neuron_add_out_synapse(in_neuron, synapse);
        }
    }
    
    status = TRUE;
    error:
    // NOTE: NO NEED TO DEALLOCATE THE SYNAPSES THAT WERE SUCCESFULL
    // NOTE: BECAUSE IF THIS FAILS WE WOULD DELETE THE LAYER WHICH
    // NOTE: DELETES THE NEURONS WHICH DELETE THE SYNAPSES
    return status;
}


internal bool
layer_link(Layer* layer, Layer* input_layer) {
    bool status = FALSE;
    check(layer != NULL, "layer is NULL");
    check(input_layer != NULL, "input_layer is NULL");
    
    if (layer->type == LAYER_DENSE) {
        status = layer_link_dense(layer, input_layer);
    }
    
    error:
    return status;
}
