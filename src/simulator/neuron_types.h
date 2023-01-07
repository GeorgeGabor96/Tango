#ifndef NEURON_TYPES
#define NEURON_TYPES


typedef enum { 
    NEURON_INVALID,
    NEURON_LIF,
    NEURON_LIF_REFRACT 
} NeuronType; 


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


typedef struct NeuronCls {
    String* name;  // NOTE: take ownership of the name
    NeuronType type;
    union {
        struct {
            u32 refract_time;
        } lif_refract_cls;
    };
} NeuronCls;


typedef struct InSynapseArray {
    u32 length;
    sz synapse_size;
    Synapse* synapses;
} InSynapseArray;



typedef struct OutSynapseArray {
    u32 length;
    Synapse** synapses;
} OutSynapseArray;


#define NEURON_N_MAX_INPUTS 5u
#define NEURON_N_MAX_OUTPUTS 5u
#define NEURON_INVALID_SPIKE_TIME -1


typedef struct Neuron {
    NeuronCls* cls;
    f32 voltage;
    f32 epsc;
    f32 ipsc;
    bool spike;
    u32 last_spike_time;
    
    // NOTE: the neuron owns its input synapses
    InSynapseArray* in_synapse_arrays[NEURON_N_MAX_INPUTS];
    OutSynapseArray* out_synapse_arrays[NEURON_N_MAX_OUTPUTS];
    
    u32 n_in_synapse_arrays;
    u32 n_out_synapse_arrays;
    
    union {
        struct {
            u32 last_spike_time;
        } lif_refract;
    };
} Neuron;


#endif // NEURON_TYPES

