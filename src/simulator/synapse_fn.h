#ifndef SYNAPSE_FN_H
#define SYNAPSE_FN_H

#include "common.h"
#include "math_ops.h"
#include "utils/memory.h"
#include "utils/timing.h"
#include "containers/string.h"
#include "containers/memory_arena.h"
#include "simulator/state.h"
#include "simulator/neuron_types.h"


/*****************
* SYNAPSE_TYPE FN
*****************/
internal char* synapse_type_get_c_str(SynapseType type);


/****************
* SYNAPSE_CLS FN
****************/
internal SynapseCls* synapse_cls_create(State* state,
                                        const char* name, SynapseType type,
                                        f32 rev_potential, f32 amp, 
                                        f32 tau_ms, u32 delay);


/************
* SYNAPSE FN
************/
internal sz synapse_size_with_cls(SynapseCls* cls);
internal Synapse* synapse_create(State* state, SynapseCls* cls, f32 weight);
internal void synapse_init(Synapse* synapse, SynapseCls* cls, f32 weight);

internal u32 synapse_next_spike_time(Synapse* synapse);
internal void synapse_add_spike_time(Synapse* synapse, u32 spike_time);
internal f32 synapse_compute_psc(Synapse* synapse, f32 voltage);
internal void synapse_step(Synapse* synapse, u32 time);

internal void synapse_clear(Synapse* synapse);

// NOTE: for STDP
internal void synapse_stdp_potentiation_update(Synapse* synapse);
internal void synapse_stdp_depression_update(Synapse* synapse);

#endif //SYNAPSE_FN_H
