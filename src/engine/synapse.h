#ifndef __ENGINE_SYNAPSE_H__
#define __ENGINE_SYNAPSE_H__


typedef enum {
    SYNAPSE_LEARNING_NO_LEARNING,
    SYNAPSE_LEARNING_EXPONENTIAL,
    SYNAPSE_LEARNING_STEP,
    SYNAPSE_LEARNING_RSTDP_EXPONENTIAL,

    SYNAPSE_LEARNING_INVALID,
} SynapseLearningRule;


internal const char* synapse_learning_rule_get_c_str(SynapseLearningRule rule);


typedef struct SynapseLearningExponential {
    f32 A;
    f32 B;
    f32 tau;
} SynapseLearningExponential;


typedef struct SynapseLearningStep {
    u32 max_time_p; // NOTE: considered positive
    f32 amp_p;
    u32 max_time_d; // NOTE: considered negative
    f32 amp_d;
} SynapseLearningStep;

typedef struct SynapseLearningRSTDPExpeonential
{
    f32 reward_potentiation_factor;
    f32 reward_depression_factor;
    f32 punishment_potentiation_factor;
    f32 punishment_depression_factor;
} SynapseLearningRSTDPExpeonential;

typedef struct LearningInfo {
    SynapseLearningRule type;
    b32 enable;
    f32 max_w;
    f32 min_w;
    union {
        SynapseLearningExponential stdp_exponential;
        SynapseLearningStep stdp_step;
        SynapseLearningRSTDPExpeonential r_stdp_exponential;
    };
} LearningInfo;


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

    LearningInfo learning_info;
} SynapseCls;


internal SynapseCls* synapse_cls_create(Memory* memory,
                                        String* name, SynapseType type,
                                        f32 rev_potential, f32 amp,
                                        f32 tau_ms, u32 delay);

internal SynapseCls* synapse_cls_create_exci(Memory* memory,
                                             String* name,
                                             SynapseType type,
                                             u32 delay);

internal SynapseCls* synapse_cls_create_inhi(Memory* memory,
                                             String* name,
                                             SynapseType type,
                                             u32 delay);

internal void synapse_cls_add_learning_rule_exponential(
    SynapseCls* cls,
    f32 min_w, f32 max_w,
    f32 A, f32 B, f32 tau);

internal void synapse_cls_add_learning_rule_step(
    SynapseCls* cls,
    f32 min_w, f32 max_w,
    u32 max_time_p, f32 amp_p,
    u32 max_time_d, f32 amp_d);

internal void syanpse_cls_add_learning_rule_rstdp_exponential(
    SynapseCls* cls,
    f32 min_w,
    f32 max_w,
    f32 reward_potentiation_factor,
    f32 reward_depression_factor,
    f32 punishment_potentiation_factor,
    f32 punishment_depression_factor);


#define SYNAPSE_QUEUE_CAPACITY (sizeof(u64) * 8 - 1)

struct Synapse {
    SynapseCls* cls;
    f32 weight;
    f32 conductance;
    b32 spike;
    u32 last_spike_time;

    u64 spike_queue;  // NOTE: Support delays of at most 63
                      // NOTE: Need to use a bigger type if we want a bigger delay

    Neuron* in_neuron;
    Neuron* out_neuron;
};


internal Synapse* synapse_create(Memory* memory, SynapseCls* cls, f32 weight);
internal void synapse_init(Synapse* synapse, SynapseCls* cls, f32 weight);

internal void synapse_add_spike_time(Synapse* synapse, u32 spike_time);
internal f32 synapse_compute_psc(Synapse* synapse, f32 voltage);
internal void synapse_step(Synapse* synapse, u32 time);

internal void synapse_clear(Synapse* synapse);

#endif // __SYNAPSE_H__
