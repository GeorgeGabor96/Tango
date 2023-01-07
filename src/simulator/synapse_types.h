#ifndef SYNAPSE_TYPES_H
#define SYNAPSE_TYPES_H


typedef enum {
    SYNAPSE_INVALID,
    SYNAPSE_CONDUCTANCE,
    SYNAPSE_VOLTAGE,
} SynapseType;


typedef struct SynapseCls {
    String* name;  // NOTE: take ownership of the name
    SynapseType type;
    f32 rev_potential;
    f32 amp;
    f32 tau_exp;
    u32 delay;
} SynapseCls;


typedef struct Synapse {
    SynapseCls* cls;
    f32 weight;
    f32 conductance;
    
    u32 n_max_spike_times;
    u32 n_spike_times;
    u32 spike_times_head;  // remove
    u32 spike_times_tail;  // add
    u32* spike_times;      // Directly after the synapse
    
    // TODO: need for STDP - DO I really need? 
    Neuron* in_neuron;
    Neuron* out_neuron;
} Synapse;


#endif // SYNAPSE_TYPES_H
