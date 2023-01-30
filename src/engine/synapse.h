#ifndef __ENGINE_SYNAPSE_H__
#define __ENGINE_SYNAPSE_H__


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


#define SYNAPSE_QUEUE_CAPACITY (sizeof(u64) * 8 - 1)

struct Synapse {
    SynapseCls* cls;
    f32 weight;
    f32 conductance;

    u64 spike_queue;  // NOTE: Support delays of at most 63
                      // NOTE: Need to use a bigger type if we want a bigger delay

    u32 in_neuron_i;
    u32 out_neuron_i;
};


internal Synapse* synapse_create(State* state, SynapseCls* cls, f32 weight);
internal void synapse_init(Synapse* synapse, SynapseCls* cls, f32 weight);

internal void synapse_add_spike_time(Synapse* synapse, u32 spike_time);
internal f32 synapse_compute_psc(Synapse* synapse, f32 voltage);
internal void synapse_step(Synapse* synapse, u32 time);

internal void synapse_clear(Synapse* synapse);

// NOTE: for STDP
#define SYNAPSE_WEIGHT_MAX 2.0f

#define SYNAPSE_POTENTIATION_INTERVAL (u32)20
#define SYNAPSE_DEPRESSION_INTERVAL (u32) 30

internal void synapse_stdp_potentiation_update(Synapse* synapse, u32 in_spike_time, u32 out_spike_time);

internal void synapse_stdp_depression_update(Synapse* synapse, u32 in_spike_time, u32 out_spike_time);

#endif // __SYNAPSE_H__
