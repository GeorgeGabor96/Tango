#ifndef __ENGINE_TYPES_H__
#define __ENGINE_TYPES_H__


typedef struct Neuron Neuron;
typedef struct Synapse Synapse;
typedef struct Synapse* SynapseP;


typedef enum {
    MODE_INFER,
    MODE_LEARNING,
    MODE_INVALID
} Mode;

internal const
char* mode_get_c_str(Mode mode) {
    if (mode == MODE_INFER) return "MODE_INFER";
    else if (mode == MODE_LEARNING) return "MODE_LEARNING";
    else return "MODE_INVALID";
}

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
    bool* spikes;
    u32 n_spikes;
} Spikes;


typedef struct Input {
    InputType type;
    union {
        Spikes spikes;
        Currents currents;
    };
} Input;

typedef struct Inputs {
    Input* inputs;
    u32 n_inputs;
} Inputs;


#endif // __ENGINE_TYPES_H__
