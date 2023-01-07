#ifndef NEURON_FN_H
#define NEURON_FN_H

#include "common.h"
#include "utils/memory.h"
#include "containers/string.h"
#include "containers/memory_arena.h"
#include "containers/array.h"
#include "simulator/neuron_types.h"
#include "simulator/synapse_types.h"


/***************
* NEURON_CLS FN
***************/
internal char* neuron_type_get_c_str(NeuronType type);

internal NeuronCls* neuron_cls_create_lif(State* state, const char* name);
internal NeuronCls* neuron_cls_create_lif_refract(State* state, 
                                                  const char* name,
                                                  u32 refract_time);


/*********************
* IN_SYNAPSE_ARRAY FN
*********************/
internal Synapse* in_synapse_array_get(InSynapseArray* synapses, u32 i);


/***********
* NEURON FN
***********/
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

#endif //NEURON_FN_H

