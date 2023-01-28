#ifndef __ENGINE_NEURON_H__
#define __ENGINE_NEURON_H__


typedef enum {
    NEURON_INVALID,
    NEURON_LIF,
    NEURON_LIF_REFRACT
} NeuronType;


internal char* neuron_type_get_c_str(NeuronType type);


typedef struct NeuronCls {
    String* name;  // NOTE: take ownership of the name
    NeuronType type;
    union {
        struct {
            u32 refract_time;
        } lif_refract_cls;
    };
} NeuronCls;


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


internal NeuronCls* neuron_cls_create_lif(State* state, const char* name);
internal NeuronCls* neuron_cls_create_lif_refract(State* state,
                                                  const char* name,
                                                  u32 refract_time);

typedef struct SynapseIdxArray {
    u32 capacity;
    u32 length;
    u32* idxs;
    struct SynapseIdxArray* next;
} SynapseIdxArray;


#define NEURON_N_MAX_INPUTS 5u
#define NEURON_N_MAX_OUTPUTS 5u
#define NEURON_INVALID_SPIKE_TIME (u32)-1


struct Neuron {
    NeuronCls* cls;
    f32 voltage;
    f32 epsc;
    f32 ipsc;
    bool spike;
    u32 last_spike_time;

    SynapseIdxArray* in_synapse_arrays;
    SynapseIdxArray* out_synapse_arrays;
    u32 n_in_synapse_arrays;
    u32 n_out_synapse_arrays;
};


internal Neuron* neuron_create(State* state,  NeuronCls* cls);
internal void neuron_init(Neuron* neuron, NeuronCls* cls);

internal void neuron_step(Neuron* neuron, Synapse* synapses, u32 time);

// NOTE: use to set inputs
internal void neuron_step_force_spike(Neuron* neuron, Synapse* synapses, u32 time);
internal void neuron_step_inject_current(Neuron* neuron, Synapse* synapses, f32 psc, u32 time);

internal void neuron_clear(Neuron* neuron, Synapse* synapses);

// For plasticity
internal void neuron_learning_step(Neuron* neuron, Neuron* neurons, Synapse* synapses, u32 time);
internal void neuron_learning_step_force_spike(Neuron* neuron, Neuron* neurons, Synapse* synapses, u32 time);
internal void neuron_learning_step_inject_current(Neuron* neuron, Neuron* neurons, Synapse* synapses,
                                                  f32 psc,
                                                  u32 time);

#endif // __ENGINE_NEURON_H__

