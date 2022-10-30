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
layer_create(const char* name, LayerType type, u32 n_neurons, NeuronCls* cls) {
    Layer* layer = NULL;
    bool flag = FALSE;
    
    layer = (Layer*) memory_malloc(sizeof(*layer), "layer_create");
    check_memory(layer);
    
    flag = layer_init(layer, name, type, n_neurons, cls);
    check(flag == TRUE, "couldn't init the layer");
    
    return layer;
    
    error:
    memory_free(layer);
    return NULL;
}


internal bool
layer_init(Layer* layer, const char* name, LayerType type, 
           u32 n_neurons, NeuronCls* cls) {
    u32 i = 0;
    bool status = FALSE;
    
    check(layer != NULL, "layer is NULL");
    check(name != NULL, "name is NULL");
    check(type == LAYER_DENSE, "invalid layer type %s", 
          layer_type_get_c_str(type));
    check(cls != NULL, "cls is NULL");
    
    
    layer->n_neurons = n_neurons;
    layer->neurons = (Neuron*)memory_malloc(layer->n_neurons * sizeof(Neuron),
                                            "layer_init layer->neurons");
    check_memory(layer->neurons);
    for (i = 0; i < layer->n_neurons; ++i) {
        status = neuron_init(layer->neurons + i, cls);
        check(status == TRUE, "couldn't init neuron %u", i);
    }
    
    layer->n_in_layers = 0;
    layer->n_max_in_layers = 5;
    layer->in_layers = (Layer**) memory_malloc(layer->n_max_in_layers * sizeof(Layer*),
                                               "layer_init layer->in_layers");
    check_memory(layer->in_layers);
    
    layer->name = string_create(name);
    check_memory(layer->name);
    layer->type = type;
    layer->it_ran = FALSE;
    
    // TODO:
    if (layer->type == LAYER_DENSE) {
        
    }
    
    return TRUE;
    error:
    if (layer->neurons != NULL) {
        for (u32 j = 0; j < i; ++j)
            neuron_reset(layer->neurons + j);
        memory_free(layer->neurons);
    }
    if (layer->in_layers != NULL)
        memory_free(layer->in_layers);
    if (layer->name != NULL)
        string_destroy(layer->name);
    
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
    string_destroy(layer->name);
    memory_free(layer->neurons);
    memory_free(layer->in_layers);
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
    check(layer != NULL, "layer is NULL");
    
    for (u32 i = 0; i < layer->n_neurons; ++i)
        neuron_step(layer->neurons + i, time);
    
    error:
    return;
}


internal void
layer_step_inject_current(Layer* layer, u32 time, f32* currents, u32 n_currents) {
    check(layer != NULL, "layer is NULL");
    check(currents != NULL, "currents is NULL");
    
    u32 n_inputs = math_min_u32(layer->n_neurons, n_currents);
    u32 i = 0;
    for (i = 0; i < n_inputs; ++i) 
        neuron_step_inject_current(layer->neurons + i, currents[i], time);
    for (i = n_inputs; i < layer->n_neurons; ++i){
        neuron_step(layer->neurons + i, time);
    }
    error:
    return;
}


internal void
layer_step_force_spike(Layer* layer, u32 time, bool* spikes, u32 n_spikes) {
    check(layer != NULL, "layer is NULL");
    check(spikes != NULL, "spikes is NULL");
    
    u32 n_inputs = math_min_u32(layer->n_neurons, n_spikes);
    u32 i = 0;
    for (i = 0; i < n_inputs; ++i) {
        if (spikes[i] == TRUE) 
            neuron_step_force_spike(layer->neurons + i, time);
        else
            neuron_step(layer->neurons + i, time);
    }
    for (i = n_inputs; i < layer->n_neurons; ++i)
        neuron_step(layer->neurons + i, time);
    
    error:
    return;
}


internal void
layer_clear(Layer* layer) {
    check(layer != NULL, "layer is NULL");
    
    for (u32 i = 0; i < layer->n_neurons; ++i)
        neuron_clear(layer->neurons + i);
    
    layer->it_ran = FALSE;
    
    error:
    return;
}


internal void
layer_show(Layer* layer) {
    check(layer != NULL, "layer is NULL");
    
    u32 i = 0;
    u32 j = 0;
    u32 n_in_synapses = 0;
    Neuron* neuron = NULL;
    for (i = 0; i < layer->n_neurons; ++i) {
        neuron = layer->neurons + i;
        for (j = 0; j < neuron->n_in_arrays; ++j) {
            n_in_synapses = neuron->in_arrays[j]->length;
        }
    }
    printf("-------------------\n");
    printf("Name: %s\n", string_to_c_str(layer->name));
    printf("Type: %s\n", layer_type_get_c_str(layer->type));
    printf("Number of neurons %u of type %s\n",
           layer->n_neurons,
           string_to_c_str(layer->neurons[0].cls->name));
    printf("Number of input synapses %u\n", n_in_synapses);
    printf("Input layers: ");
    for (i = 0; i < layer->n_in_layers; ++i)
        printf("%s, ", string_to_c_str(layer->in_layers[i]->name));
    
    printf("\n-------------------\n\n");
    error:
    return;
}


inline internal bool
layer_link_dense(Layer* layer, Layer* in_layer,
                 SynapseCls* cls, f32 weight) {
    u32 neuron_i = 0;
    u32 in_neuron_i = 0;
    u32 synapse_i = 0;
    Neuron* neuron = NULL;
    Neuron* in_neuron = NULL;
    Synapse* synapse = NULL;
    SynapseArray* synapses = NULL;
    bool status = FALSE;
    
    for (neuron_i = 0; neuron_i < layer->n_neurons; ++neuron_i) {
        neuron = layer->neurons + neuron_i;
        
        // ALLOC all input synapses
        synapses = (SynapseArray*)memory_malloc(sizeof(SynapseArray) + sizeof(Synapse) *  in_layer->n_neurons,
                                                "layer_link_dense synapses");
        check_memory(synapses);
        synapses->max_length = in_layer->n_neurons;
        synapses->length = in_layer->n_neurons;
        synapses->data = (Synapse*)(synapses + 1);
        synapse_i = 0;
        
        for (in_neuron_i = 0; in_neuron_i < in_layer->n_neurons; ++in_neuron_i) {
            in_neuron = in_layer->neurons + in_neuron_i;
            
            synapse = synapses->data + synapse_i;
            ++synapse_i;
            
            synapse_init(synapse, cls, weight);
            neuron_add_out_synapse(in_neuron, synapse);
        }
        neuron_add_in_synapse_array(neuron, synapses);
    }
    
    status = TRUE;
    error:
    // NOTE: NO NEED TO DEALLOCATE THE SYNAPSES THAT WERE SUCCESFULL
    // NOTE: BECAUSE IF THIS FAILS WE WOULD DELETE THE LAYER WHICH
    // NOTE: DELETES THE NEURONS WHICH DELETE THE SYNAPSES
    return status;
}


internal bool
layer_link(Layer* layer, Layer* input_layer, SynapseCls* cls, f32 weight) {
    bool status = FALSE;
    check(layer != NULL, "layer is NULL");
    check(input_layer != NULL, "input_layer is NULL");
    
    if (layer->type == LAYER_DENSE) {
        status = layer_link_dense(layer, input_layer, cls, weight);
        check(status == TRUE, "couldn't link layers %s and %s",
              string_to_c_str(layer->name), string_to_c_str(input_layer->name));
    } else {
        log_error("Unknown layer type %u", layer->type);
    }
    
    // NOTE: Save reference to the input layer
    if (layer->n_in_layers == layer->n_max_in_layers) {
        u32 new_n_max_in_layers = layer->n_max_in_layers * 2;
        layer->in_layers = array_resize(layer->in_layers, sizeof(Layer*),
                                        layer->n_in_layers,
                                        new_n_max_in_layers);
        check(layer->in_layers != NULL, "layer->in_layers is NULL");
        layer->n_max_in_layers = new_n_max_in_layers;
    }
    layer->in_layers[layer->n_in_layers] = input_layer;
    ++(layer->n_in_layers);
    
    error:
    return status;
}


internal f32*
layer_get_voltages(Layer* layer) {
    check(layer != NULL, "layer is NULL");
    
    f32* voltages = (f32*)memory_malloc(sizeof(f32) * layer->n_neurons, "layer_get_voltages");
    check_memory(voltages);
    
    for (u32 i = 0; i < layer->n_neurons; ++i)
        voltages[i] = layer->neurons[i].voltage;
    
    return voltages;
    
    error:
    return NULL;
}


internal f32*
layer_get_pscs(Layer* layer) {
    check(layer != NULL, "layer is NULL");
    
    f32* pscs = (f32*)memory_malloc(sizeof(f32) * layer->n_neurons, "layer_get_pscs");
    check_memory(pscs);
    
    for (u32 i = 0; i < layer->n_neurons; ++i)
        pscs[i] = layer->neurons[i].epsc + layer->neurons[i].ipsc;
    
    return pscs;
    
    error:
    return NULL;
}


internal f32*
layer_get_epscs(Layer* layer) {
    check(layer != NULL, "layer is NULL");
    
    f32* epscs = (f32*)memory_malloc(sizeof(f32) * layer->n_neurons, "layer_get_epscs");
    check_memory(epscs);
    
    for (u32 i = 0; i < layer->n_neurons; ++i)
        epscs[i] = layer->neurons[i].epsc;
    
    return epscs;
    
    error:
    return NULL;
}


internal f32*
layer_get_ipscs(Layer* layer) {
    check(layer != NULL, "layer is NULL");
    
    f32* ipscs = (f32*)memory_malloc(sizeof(f32) * layer->n_neurons, "layer_get_ipscs");
    check_memory(ipscs);
    
    for (u32 i = 0; i < layer->n_neurons; ++i)
        ipscs[i] = layer->neurons[i].ipsc;
    
    return ipscs;
    
    error:
    return NULL;
}


internal bool*
layer_get_spikes(Layer* layer) {
    check(layer != NULL, "layer is NULL");
    
    bool* spikes = (bool*)memory_malloc(sizeof(bool) * layer->n_neurons, "layer_get_spikes");
    check_memory(spikes);
    
    for (u32 i = 0; i < layer->n_neurons; ++i)
        spikes[i] = layer->neurons[i].spike;
    
    return spikes;
    
    error:
    return NULL;
}
