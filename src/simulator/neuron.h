/* date = September 15th 2022 8:05 pm */

#ifndef NEURON_H
#define NEURON_H

#include "common.h"
#include "utils/memory.h"
#include "containers/string.h"
#include "containers/memory_arena.h"
#include "containers/array.h"
#include "simulator/synapse.h"


typedef enum { 
    NEURON_INVALID,
    NEURON_LIF,
    NEURON_LIF_REFRACT 
} NeuronType; 

internal char* neuron_type_get_c_str(NeuronType type);

// NOTE: LIF constants
// TODO: This change very rarely for now, only when we have a config file of some sort we
// TODO: may think of using globals or per class values
#define NEURON_LIF_VOLTAGE_TH -45.0f
#define NEURON_LIF_VOLTAGE_REST -65.0f
#define NEURON_LIF_R 10.0f
#define NEURON_LIF_C 1.0f
#define NEURON_LIF_TAU NEURON_LIF_R * NEURON_LIF_C
#define NEURON_LIF_VOLTAGE_FACTOR (1 - 1 / (NEURON_LIF_TAU))
#define NEURON_LIF_I_FACTOR (1 / NEURON_LIF_C)
#define NEURON_LIF_FREE_FACTOR (NEURON_LIF_VOLTAGE_REST / (NEURON_LIF_TAU))


typedef struct NeuronCls {
    String* name;  // NOTE: take ownership of the name
    NeuronType type;
    union {
        struct {
            u32 refract_time;
        } lif_refract_cls;
    };
} NeuronCls;


internal NeuronCls* neuron_cls_create_lif(State* state, const char* name);
internal NeuronCls* neuron_cls_create_lif_refract(State* state, 
                                                  const char* name,
                                                  u32 refract_time);

typedef struct InSynapseArray {
    u32 length;
    sz synapse_size;
    Synapse* synapses;
} InSynapseArray;


typedef struct OutSynapseArray {
    u32 length;
    Synapse** synapses;
} OutSynapseArray;


#define NEURON_N_MAX_INPUTS 5
#define NEURON_N_MAX_OUTPUTS 5


typedef struct Neuron {
    NeuronCls* cls;
    f32 voltage;
    f32 epsc;
    f32 ipsc;
    bool spike;
    
    // NOTE: the neuron owns its input synapses
    InSynapseArray* in_synapse_arrays[NEURON_N_MAX_INPUTS];
    OutSynapseArray* out_synapse_arrays[NEURON_N_MAX_OUTPUTS];
    
    u32 n_in_synapse_arrays;
    u32 n_out_synapse_arrays;
    
    union {
        struct {
            u32 last_spike_time;
        } lif_refract;
    };
} Neuron;


internal Neuron* neuron_create(State* state,  NeuronCls* cls);
internal void neuron_init(Neuron* neuron, NeuronCls* cls);

// NOTE: the neuron takes ownership over the synapses array
internal void neuron_add_in_synapse_array(Neuron* neuron,
                                          InSynapseArray* synapses);
internal void neuron_add_out_synapse_array(Neuron* neuron, OutSynapseArray* synapses);

internal void neuron_step(Neuron* neuron, u32 time);

// NOTE: use to set inputs
internal void neuron_step_force_spike(Neuron* neuron, u32 time);
internal void neuron_step_inject_current(Neuron* neuron, f32 psc, u32 time);

internal void neuron_clear(Neuron* neuron);

#endif //NEURON_H
