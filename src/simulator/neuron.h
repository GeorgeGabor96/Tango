/* date = September 15th 2022 8:05 pm */

#ifndef NEURON_H
#define NEURON_H

#include "common.h"
#incldue "containers/array.h"


typedef enum { 
    NEURON_INVALID,
    NEURON_LIF,
    NEURON_LIF_REFRACT 
} NeuronType; 

internal char* neuron_type_get_c_str(NeuronType type);

// NOTE: LIF constants
// TODO: This change very rarely for now, only when we have a config file of some sort we
// TODO: may think of using globals or per class values
#define NEURON_LIF_U_TH -45.0f
#define NEURON_LIF_U_REST -65.0f
#define NEURON_LIF_R 10.0f
#define NEURON_LIF_C 1.0f
#define NEURON_LIF_TAU NEURON_LIF_R * NEURON_LIF_C
#define NEURON_LIF_U_FACTOR (1 - 1 / (NEURON_LIF_TAU))
#define NEURON_LIF_I_FACTOR (1 / NEURON_LIF_C)
#define NEURON_LIF_FREE_FACTOR (NEURON_LIF_U_REST / (NEURON_LIF_TAU))


typedef struct NeuronCls {
    NeuronType type;
    union data {
        struct lif_refract {
            u32 refract_time;
        };
    };
} NeuronCls;


internal NeuronClass* neuron_cls_create_lif();
internal NeuronClass* neuron_cls_create_lif_refract(u32 refract_time);
internal void neuron_cls_destroy(NeuronCls* cls);

internal void neuron_cls_move(NeuronCls* cls_src, NeuronCls* cls_dst);


typedef struct Neuron {
    NeuronCls* cls;
    f32 voltage;
    f32 epsc;
    f32 ipsc;
    Array* in_synapses_ref;   // NOTE: keep references 
    Array* out_synapses_ref;  // NOTE: keep references
    bool spike;
    
    union data {
        struct lif_refract {
            u32 last_spike_time;
        };
    };
} Neuron;


// TODO: Should we give the synapses directly or one by one as in the old aproach????
// TODO: the answer will appear when writting the layers or network
internal Neuron* neuron_create(NeuronCls* cls, 
                               Array* in_synapses_ref, 
                               Array* out_synapses_ref);
internal void neuron_destroy(Neuron* neuron);

// TODO: how to move a neuron
// internal void neuron_move(Neuron* neuron_src, Neuron* neuron_dst);

// TODO: do I need to add the synapses one by one?
//internal void neuron_add_in_synapse(Neuron* neuron, Synapse* synapse);
//internal void neuron_add_out_synapse(Neuron* neuron, Synapse* synapse);

internal void neuron_step(Neuron* neuron, u32 time);
internal void neuron_step_force_spike(Neuron* neuron, u32 time);
internal void neuron_step_inject_current(Neuron* neuron, f32 psc, u32 time);




#endif //NEURON_H
