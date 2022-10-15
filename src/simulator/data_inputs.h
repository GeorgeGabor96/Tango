/* date = October 15th 2022 3:15 pm */

#ifndef DATA_INPUTS_H
#define DATA_INPUTS_H

#include "common.h"
#include "simulator/data_gen.h"
#include "simulator/network.h"


typedef enum {
    DATA_INPUT_INVALID,
    DATA_INPUT_SPIKES,
    DATA_INPUT_CURRENT,
} DataInputType;


typedef struct DataInput {
    DataInputType type;
    void* data;
    u32 n_values;
} DataInput;


typedef struct DataInputs {
    DataInput* inputs;
    u32 n_inputs;
} DataInputs;


// NOTE: Currently the easiest thing to do is to give the network when creating inputs
// NOTE: Probably in the future this is not enough but we will see
internal DataInputs* data_gen_inputs_create(DataSample* sample, Network* network, u32 time);
internal void data_gen_inputs_destroy(DataInputs* inputs);


#endif //DATA_INPUTS_H
