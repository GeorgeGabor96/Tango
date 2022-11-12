/* date = September 16th 2022 7:26 pm */

#ifndef SYNAPSE_H
#define SYNAPSE_H

#include "common.h"
#include "math_ops.h"
#include "utils/memory.h"
#include "utils/timing.h"
#include "containers/string.h"
#include "containers/memory_arena.h"
#include "simulator/state.h"


typedef enum {
    SYNAPSE_INVALID,
    SYNAPSE_CONDUCTANCE,
    SYNAPSE_VOLTAGE,
} SynapseType;

internal char* synapse_type_get_c_str(SynapseType type);


typedef struct SynapseCls {
    String* name;  // NOTE: take ownership of the name
    SynapseType type;
    f32 rev_potential;
    f32 amp;
    f32 tau_exp;
    u32 delay;
} SynapseCls;


internal SynapseCls* synapse_cls_create(State* state,
                                        const char* name, SynapseType type,
                                        f32 rev_potential, f32 amp, 
                                        f32 tau_ms, u32 delay);

typedef struct Synapse {
    SynapseCls* cls;
    f32 weight;
    f32 conductance;
    
    u32 n_max_spike_times;
    u32 n_spike_times;
    u32 spike_times_head;  // remove
    u32 spike_times_tail;  // add
    u32* spike_times;      // Directly after the synapse
} Synapse;


internal sz synapse_size_with_cls(SynapseCls* cls);
internal Synapse* synapse_create(State* state, SynapseCls* cls, f32 weight);
internal void synapse_init(Synapse* synapse, SynapseCls* cls, f32 weight);

internal u32 synapse_next_spike_time(Synapse* synapse);
internal void synapse_add_spike_time(Synapse* synapse, u32 spike_time);
internal f32 synapse_compute_psc(Synapse* synapse, f32 voltage);
internal void synapse_step(Synapse* synapse, u32 time);

internal void synapse_clear(Synapse* synapse);

#endif //SYNAPSE_H
