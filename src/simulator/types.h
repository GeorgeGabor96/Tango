#ifndef __TYPES_H__
#define __TYPES_H__

#include "common.h"

typedef struct Neuron Neuron;
typedef struct Synapse Synapse;


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


#endif // __TYPES_H__ 
