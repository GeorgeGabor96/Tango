/* date = September 16th 2022 7:26 pm */

#ifndef SYNAPSE_H
#define SYNAPSE_H

#include "common.h"
#include "containers/string.h"
#include "math_ops.h"


typedef enum {
    SYNAPSE_INVALID,
    SYNAPSE_CONDUCTANCE,
    SYNAPSE_VOLTAGE
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


internal SynapseCls* synapse_cls_create(String* name, SynapseType type,
                                        f32 rev_potential, f32 amp, 
                                        f32 tau_ms, u32 delay);
internal void synapse_cls_destroy(SynapseCls* cls);

internal void synapse_cls_move(SynapseCls* cls_src, SynapseCls* cls_dst);


typedef struct Synapse {
    SynapseCls* cls;
    f32 weight;
    f32 conductance;
    
    u32 n_max_spike_times;
    u32 n_spike_times;
    u32 spike_times_head;  // remove
    u32 spike_times_tail;  // add
    u32* spike_times;
} Synapse, *SynapseP;


internal Synapse* synapse_create(SynapseCls* cls, f32 weight);
internal bool synapse_init(Synapse* synapse, SynapseCls* cls, f32 weight);
internal void synapse_destroy(Synapse* synapse);
internal void synapse_reset(Synapse* synapse);

// TODO: how to move a synapse?
// internal void synapse_move(Synapse* synapse_src, Synapse* synapse_dst);

internal u32 synapse_next_spike_time(Synapse* synapse);
internal void synapse_add_spike_time(Synapse* synapse, u32 spike_time);
internal f32 synapse_compute_psc(Synapse* synapse, f32 voltage);
internal void synapse_step(Synapse* synapse, u32 time);


#endif //SYNAPSE_H
