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
layer_create(State* state, const char* name, LayerType type, u32 n_neurons, NeuronCls* cls) {
    Layer* layer = NULL;
    u32 i = 0;
    
    check(state != NULL, "state is NULL");
    check(name != NULL, "name is NULL");
    check(type == LAYER_DENSE, "invalid layer type %s", 
          layer_type_get_c_str(type));
    check(n_neurons > 0, "n_neurons is 0");
    check(cls != NULL, "cls is NULL");
    
    layer = (Layer*) memory_arena_push(state->permanent_storage, sizeof(*layer));
    check_memory(layer);
    
    layer->n_neurons = n_neurons;
    // TODO: should I just keep pointers to neurons and remove the init?
    layer->neurons = (Neuron*)
        memory_arena_push(state->permanent_storage, layer->n_neurons * sizeof(Neuron));
    check_memory(layer->neurons);
    for (i = 0; i < layer->n_neurons; ++i)
        neuron_init(layer->neurons + i, cls);
    
    memset(layer->in_layers, 0, sizeof(Layer*) * LAYER_N_MAX_IN_LAYERS);
    
    layer->name = string_create(state->permanent_storage, name);
    check_memory(layer->name);
    layer->type = type;
    layer->it_ran = FALSE;
    
    return layer;
    error:
    return NULL;
}


internal void
layer_step(Layer* layer, u32 time,
           MemoryArena* transient_storage, u32 n_cpus) {
    TIMING_COUNTER_START(LAYER_STEP);
    
    check(layer != NULL, "layer is NULL");
    check(n_cpus != 0, "n_cpus is 0");
    
    // NOTE: this I think can be allocated in the create so that no need to pass arena
    ThreadPool pool;
    pool.orders = (ThreadWorkOrder*)
        memory_arena_push(transient_storage, n_cpus * sizeof(ThreadWorkOrder)); 
    pool.n_orders = n_cpus;
    pool.c_order_idx = 0;
    
    // TODO: Its ugly that in this way I will create the threads here every time
    // TODO: can't I create the threads in the layer create and keep them and 
    // TODO: give them work from time to time???
    
    // NOTE: Create n_cpus orders that need to be ran in threads
    u32 n_neurons_per_order = (layer->n_neurons + n_cpus - 1) / n_cpus;
    for (u32 order_i = 0; order_i < n_cpus; ++order_i) {
        ThreadWorkOrder* order = pool.orders + order_i;
        order->layer = layer;
        order->neuron_idx_start = order_i * n_neurons_per_order;
        order->neuron_idx_end = order->neuron_idx_start + n_neurons_per_order;
        if (order->neuron_idx_end > layer->n_neurons)
            order->neuron_idx_end = layer->n_neurons;
    }
    
    while (pool.c_order_idx < pool.n_orders) {
        // TODO: move this into a process order???
        ThreadWorkOrder* order = pool.orders + pool.c_order_idx;
        ++(pool.c_order_idx);
        log_info("Order %u: start %u, end %u, layer->n_neurons %u",
                 pool.c_order_idx, order->neuron_idx_start,
                 order->neuron_idx_end, layer->n_neurons);
        for (u32 neuron_i = order->neuron_idx_start;
             neuron_i < order->neuron_idx_end;
             ++neuron_i)
            neuron_step(layer->neurons + neuron_i, time);
    }
    
    TIMING_COUNTER_END(LAYER_STEP);
    
    error:
    return;
}


internal void
layer_step_inject_current(Layer* layer, u32 time, f32* currents, u32 n_currents) {
    TIMING_COUNTER_START(LAYER_STEP_INJECT_CURRENT);
    
    check(layer != NULL, "layer is NULL");
    check(currents != NULL, "currents is NULL");
    
    // TODO: Do I need like an order type for this also?
    
    u32 n_inputs = math_min_u32(layer->n_neurons, n_currents);
    u32 i = 0;
    for (i = 0; i < n_inputs; ++i) 
        neuron_step_inject_current(layer->neurons + i, currents[i], time);
    for (i = n_inputs; i < layer->n_neurons; ++i){
        neuron_step(layer->neurons + i, time);
    }
    
    TIMING_COUNTER_END(LAYER_STEP_INJECT_CURRENT);
    
    error:
    return;
}


internal void
layer_step_force_spike(Layer* layer, u32 time, bool* spikes, u32 n_spikes) {
    TIMING_COUNTER_START(LAYER_STEP_FORCE_SPIKE);
    
    check(layer != NULL, "layer is NULL");
    check(spikes != NULL, "spikes is NULL");
    
    // TODO: Do I need like an order type for this also?
    
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
    
    TIMING_COUNTER_END(LAYER_STEP_FORCE_SPIKE);
    
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


internal u32
layer_get_n_in_synapses(Layer* layer) {
    u32 n_in_synapses = 0;
    u32 i = 0;
    u32 j = 0;
    Neuron* neuron = NULL;
    
    check(layer != NULL, "Layer is NULL");
    
    for (i = 0; i < layer->n_neurons; ++i) {
        neuron = layer->neurons + i;
        for (j = 0; j < neuron->n_in_synapse_arrays; ++j) {
            n_in_synapses += neuron->in_synapse_arrays[j]->length;
        }
    }
    
    error:
    return n_in_synapses;
}


internal void
layer_show(Layer* layer) {
    check(layer != NULL, "layer is NULL");
    
    u32 i = 0;
    u32 j = 0;
    u32 n_in_synapses = layer_get_n_in_synapses(layer);
    
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
layer_link_dense(State* state,
                 Layer* layer, Layer* in_layer,
                 SynapseCls* cls, f32 weight) {
    u32 neuron_i = 0;
    u32 in_neuron_i = 0;
    u32 synapse_i = 0;
    Neuron* neuron = NULL;
    Neuron* in_neuron = NULL;
    Synapse* synapse = NULL;
    InSynapseArray* in_synapses = NULL;
    bool status = FALSE;
    sz synapse_size = synapse_size_with_cls(cls);
    
    OutSynapseArray* out_synapses = NULL;
    OutSynapseArray** out_synapses_in_layer = memory_arena_push(state->transient_storage,
                                                                in_layer->n_neurons * sizeof(OutSynapseArray*));
    check_memory(out_synapses_in_layer);
    
    // INIT the output synapses for each of the input neurons
    for (neuron_i = 0; neuron_i < in_layer->n_neurons; ++neuron_i) {
        out_synapses = memory_arena_push(state->permanent_storage,
                                         sizeof(OutSynapseArray) + sizeof(Synapse*) * layer->n_neurons);
        check_memory(out_synapses);
        out_synapses->length = 0;
        out_synapses->synapses = (Synapse**)(out_synapses + 1);
        out_synapses_in_layer[neuron_i] = out_synapses;
    }
    
    for (neuron_i = 0; neuron_i < layer->n_neurons; ++neuron_i) {
        neuron = layer->neurons + neuron_i;
        
        // ALLOC all input synapses
        in_synapses = (InSynapseArray*)
            memory_arena_push(state->permanent_storage,
                              sizeof(InSynapseArray) + synapse_size * in_layer->n_neurons);
        check_memory(in_synapses);
        in_synapses->length = in_layer->n_neurons;
        in_synapses->synapse_size = synapse_size;
        in_synapses->synapses = (Synapse*)(in_synapses + 1);
        
        for (in_neuron_i = 0;
             in_neuron_i < in_layer->n_neurons; 
             ++in_neuron_i) {
            in_neuron = in_layer->neurons + in_neuron_i;
            
            synapse = in_synapse_array_get(in_synapses, in_neuron_i);
            
            synapse_init(synapse, cls, weight);
            
            // NOTE: save the synapse in the output synapses of the input neuron
            out_synapses = out_synapses_in_layer[in_neuron_i];
            out_synapses->synapses[out_synapses->length] = synapse;
            ++(out_synapses->length);
            
            // THIS should never happen
            check(out_synapses->length <= layer->n_neurons,
                  "out_synapses->length > layer->n_neurons");
        }
        neuron_add_in_synapse_array(neuron, in_synapses);
    }
    
    for (neuron_i = 0; neuron_i < in_layer->n_neurons; ++neuron_i) {
        neuron = in_layer->neurons + neuron_i;
        out_synapses = out_synapses_in_layer[neuron_i];
        neuron_add_out_synapse_array(neuron, out_synapses);
    }
    
    status = TRUE;
    error:
    return status;
}


internal bool
layer_link(State* state, Layer* layer, Layer* input_layer, SynapseCls* cls, f32 weight) {
    bool status = FALSE;
    check(state != NULL, "state is NULL");
    check(layer != NULL, "layer is NULL");
    check(input_layer != NULL, "input_layer is NULL");
    check(cls != NULL, "cls is NULL");
    
    if (layer->type == LAYER_DENSE) {
        status = layer_link_dense(state, layer, input_layer, cls, weight);
        check(status == TRUE, "couldn't link layers %s and %s",
              string_to_c_str(layer->name), string_to_c_str(input_layer->name));
    } else {
        log_error("Unknown layer type %u", layer->type);
    }
    
    // NOTE: Save reference to the input layer
    check(layer->n_in_layers < LAYER_N_MAX_IN_LAYERS, "too many input layers");
    layer->in_layers[layer->n_in_layers] = input_layer;
    ++(layer->n_in_layers);
    
    error:
    return status;
}


internal f32*
layer_get_voltages(MemoryArena* arena, Layer* layer) {
    check(arena != NULL, "arena is NULL");
    check(layer != NULL, "layer is NULL");
    
    f32* voltages = (f32*)
        memory_arena_push(arena, sizeof(f32) * layer->n_neurons);
    check_memory(voltages);
    
    for (u32 i = 0; i < layer->n_neurons; ++i)
        voltages[i] = layer->neurons[i].voltage;
    
    return voltages;
    
    error:
    return NULL;
}


internal f32*
layer_get_pscs(MemoryArena* arena, Layer* layer) {
    check(arena != NULL, "arena is NULL");
    check(layer != NULL, "layer is NULL");
    
    f32* pscs = (f32*)
        memory_arena_push(arena, sizeof(f32) * layer->n_neurons);
    check_memory(pscs);
    
    for (u32 i = 0; i < layer->n_neurons; ++i)
        pscs[i] = layer->neurons[i].epsc + layer->neurons[i].ipsc;
    
    return pscs;
    
    error:
    return NULL;
}


internal f32*
layer_get_epscs(MemoryArena* arena, Layer* layer) {
    check(arena != NULL, "arena is NULL");
    check(layer != NULL, "layer is NULL");
    
    f32* epscs = (f32*)
        memory_arena_push(arena, sizeof(f32) * layer->n_neurons);
    check_memory(epscs);
    
    for (u32 i = 0; i < layer->n_neurons; ++i)
        epscs[i] = layer->neurons[i].epsc;
    
    return epscs;
    
    error:
    return NULL;
}


internal f32*
layer_get_ipscs(MemoryArena* arena, Layer* layer) {
    check(arena != NULL, "arena is NULL");
    check(layer != NULL, "layer is NULL");
    
    f32* ipscs = (f32*)
        memory_arena_push(arena, sizeof(f32) * layer->n_neurons);
    check_memory(ipscs);
    
    for (u32 i = 0; i < layer->n_neurons; ++i)
        ipscs[i] = layer->neurons[i].ipsc;
    
    return ipscs;
    
    error:
    return NULL;
}


internal bool*
layer_get_spikes(MemoryArena* arena, Layer* layer) {
    check(arena != NULL, "arena is NULL");
    check(layer != NULL, "layer is NULL");
    
    bool* spikes = (bool*)
        memory_arena_push(arena, sizeof(bool) * layer->n_neurons);
    check_memory(spikes);
    
    for (u32 i = 0; i < layer->n_neurons; ++i)
        spikes[i] = layer->neurons[i].spike;
    
    return spikes;
    
    error:
    return NULL;
}
