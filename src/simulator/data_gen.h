/* date = October 13th 2022 8:19 pm */

#ifndef DATA_GEN_H
#define DATA_GEN_H


#include "common.h"
#include "simulator/network.h"


typedef enum {
    DATA_GEN_INVALID,
    DATA_GEN_CONSTANT_CURRENT,
    DATA_GEN_RANDOM_SPIKES,
} DataGenType;


typedef struct DataGen {
    DataGenType type;
    u32 length;
    
    union {
        struct {
            f32 value;
            u32 duration;
        } gen_const_current;
        struct {
            f32 chance;
            u32 duration;
        } gen_random_spikes;
    }
} DataGen;


internal void data_gen_create_constant_current(f32 value, u32 length);
internal void data_gen_create_random_spikes(f32 chance, u32 length);
internal void data_gen_destroy(DataGen* data);


// TODO: Do i need this enum????
typedef enum {
    DATA_SAMPLE_INVALID,
    DATA_SAMPLE_RANDOM_SPIKES,
    DATA_SAMPLE_CONSTANT_CURRENT,
} DataSampleType;


typedef struct DataSample {
    DataSampleType type;
    u32 duration;
    
    union {
        struct {
            f32 chance;
        } sample_random_spikes;
        struct {
            f32 value;
        } sample_const_current;
    }
} Sample;


internal DataSample* data_gen_sample_create(DataGen* data, u32 idx);
internal void data_gen_sample_destroy(DataSample* sample);


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


#endif //DATA_GEN_H
