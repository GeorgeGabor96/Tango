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
} NeuronClass;


internal NeuronClass* neuron_class_create_lif();
internal NeuronClass* neuron_class_create_lif_refract(u32 refract_time);
internal void neuron_class_destroy(NeuronClass* cls);

internal void neuron_class_move(NeuronClass* cls_src, NeuronClass* cls_dst);


typedef struct Neuron {
    NeuronCls* cls;
    f32 u;
    f32 epsc;
    f32 ipsc;
    Array* in_synapses_ref;
    Array* out_synapses_ref;
    bool spike;
    
    union data {
        struct lif_refract {
            u32 last_spike_time;
        };
    };
} Neuron;


internal Neuron* neuron_create(NeuronClass* neuron_class);
internal void neuron_destroy(Neuron* neuron);
internal void neuron_move(Neuron* neuron_src, Neuron* neuron_dst);


#endif //NEURON_H
