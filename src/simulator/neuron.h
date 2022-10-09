/* date = September 15th 2022 8:05 pm */

#ifndef NEURON_H
#define NEURON_H

#include "common.h"
#include "containers/string.h"
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


internal NeuronCls* neuron_cls_create_lif(String* name);
internal NeuronCls* neuron_cls_create_lif_refract(String* name,
                                                  u32 refract_time);
internal void neuron_cls_destroy(NeuronCls* cls);

internal void neuron_cls_move(NeuronCls* cls_src, NeuronCls* cls_dst);


typedef struct Neuron {
    NeuronCls* cls;
    f32 voltage;
    f32 epsc;
    f32 ipsc;
    
    // NOTE: the neuron owns its input synapses
    u32 n_in_synapses;
    u32 n_max_in_synapses;
    Synapse* in_synapses;
    
    u32 n_out_synapses;
    u32 n_max_out_synapses;
    SynapseP* out_p_synapses;
    
    bool spike;
    
    union {
        struct {
            u32 last_spike_time;
        } lif_refract;
    };
} Neuron;


internal Neuron* neuron_create(NeuronCls* cls);
internal bool neuron_init(Neuron* neuron, NeuronCls* cls);
internal void neuron_destroy(Neuron* neuron);
internal void neuron_reset(Neuron* neuron);

// NOTE: When adding an input synapse take ownership and return the new address
internal Synapse* neuron_add_in_synapse(Neuron* neuron,
                                        Synapse* synapse,
                                        bool free_synapse);
internal void neuron_add_out_synapse(Neuron* neuron, Synapse* synapse);

// TODO: how to move a neuron
// internal void neuron_move(Neuron* neuron_src, Neuron* neuron_dst);

internal void neuron_step(Neuron* neuron, u32 time);

// NOTE: use to set inputs
internal void neuron_step_force_spike(Neuron* neuron, u32 time);
internal void neuron_step_inject_current(Neuron* neuron, f32 psc, u32 time);

#endif //NEURON_H
