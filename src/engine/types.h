#ifndef __ENGINE_TYPES_H__
#define __ENGINE_TYPES_H__

#define INVALID_SPIKE_TIME (u32)-1

typedef struct Neuron Neuron;
typedef struct Synapse Synapse;
typedef struct Synapse* SynapseP;

typedef enum {
    INPUT_INVALID,
    INPUT_SPIKES,
    INPUT_CURRENT,
} InputType;


typedef struct Currents {
    f32* currents;
    u32 n_currents;
} Currents;


typedef struct Spikes {
    b32* spikes;
    u32 n_spikes;
} Spikes;


typedef struct Input {
    InputType type;
    u32 label; // TODO: easiest thing to do for now, just the neuron that should win
    union {
        Spikes spikes;
        Currents currents;
    };
} Input;

typedef struct Inputs {
    Input* inputs;
    u32 n_inputs;
} Inputs;


// Data structure types
typedef struct StringNode {
    String* name;
    struct StringNode* next;
} StringNode;

typedef struct NeuronTimeSpike {
    u32 neuron_i;
    u32 time;
} NeuronTimeSpike;


#endif // __ENGINE_TYPES_H__
