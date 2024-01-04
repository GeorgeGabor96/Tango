#ifndef __ENGINE_NEURON_H__
#define __ENGINE_NEURON_H__


typedef enum {
    NEURON_IF = 0,
    NEURON_IF_REFRACT = 1,
    NEURON_IF_ONE_SPIKE = 2,
    NEURON_LIF = 3,
    NEURON_LIF_REFRACT = 4,

    NEURON_TYPES_CNT = 5,
    NEURON_INVALID,

    /*********************************
     * If adding a new type append to
     * - NEURON_VOLTAGE_REST
     * - NEURON_VOLTAGE_TH
     * - _NEURON_UPDATE_FNS
    *********************************/
} NeuronType;


// NOTE: IF constants
#define NEURON_IF_VOLTAGE_TH -45.0f
#define NEURON_IF_VOLTAGE_REST -65.0f

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

float NEURON_VOLTAGE_REST[NEURON_TYPES_CNT] = {
    NEURON_IF_VOLTAGE_REST,
    NEURON_IF_VOLTAGE_REST,
    NEURON_IF_VOLTAGE_REST,
    NEURON_LIF_VOLTAGE_REST,
    NEURON_LIF_VOLTAGE_REST
};

float NEURON_VOLTAGE_TH[NEURON_TYPES_CNT] = {
    NEURON_IF_VOLTAGE_TH,
    NEURON_IF_VOLTAGE_TH,
    NEURON_IF_VOLTAGE_TH,
    NEURON_LIF_VOLTAGE_TH,
    NEURON_LIF_VOLTAGE_TH
};

internal char* neuron_type_get_c_str(NeuronType type);


typedef struct NeuronClsIfRefract {
    u32 refract_time;
} NeuronClsIfRefract;

typedef struct NeuronClsLifRefract {
    u32 refract_time;
} NeuronClsLifRefract;

typedef struct NeuronCls {
    String* name;  // NOTE: take ownership of the name
    NeuronType type;
    union {
        NeuronClsIfRefract if_refract_cls;
        NeuronClsLifRefract lif_refract_cls;
    };
    b32 allow_learning;
} NeuronCls;


internal NeuronCls* neuron_cls_create_if(Memory* memory, String* name);
internal NeuronCls* neuron_cls_create_if_refract(Memory* memory,
                                                 String* name,
                                                 u32 refract_time);
internal NeuronCls* neuron_cls_create_if_one_spike(Memory* memory, String* name);
internal NeuronCls* neuron_cls_create_lif(Memory* memory, String* name);
internal NeuronCls* neuron_cls_create_lif_refract(Memory* memory,
                                                  String* name,
                                                  u32 refract_time);

typedef struct SynapseRefArray {
    u32 capacity;
    u32 length;
    SynapseP* synapses;
    struct SynapseRefArray* next;
} SynapseRefArray;


SynapseRefArray* neuron_create_synapses_ref_array(Memory* memory, u32 capacity);


#define NEURON_N_MAX_INPUTS 5u
#define NEURON_N_MAX_OUTPUTS 5u


struct Neuron {
    NeuronCls* cls;
    f32 voltage;
    f32 epsc;
    f32 ipsc;
    b32 spike;
    u32 last_spike_time;

    SynapseRefArray* in_synapse_arrays;
    SynapseRefArray* out_synapse_arrays;
    u32 n_in_synapse_arrays;
    u32 n_out_synapse_arrays;
};


internal Neuron* neuron_create(Memory* memory,  NeuronCls* cls);
internal void neuron_init(Neuron* neuron, NeuronCls* cls);

internal void neuron_step(Neuron* neuron, u32 time);
internal void neuron_step_force_spike(Neuron* neuron, u32 time);
internal void neuron_step_inject_current(Neuron* neuron, f32 psc, u32 time);

internal void neuron_clear(Neuron* neuron);

#endif // __ENGINE_NEURON_H__

