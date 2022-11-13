/* date = October 13th 2022 8:19 pm */

#ifndef DATA_GEN_H
#define DATA_GEN_H


#include "common.h"
#include "utils/memory.h"
#include "utils/random.h"
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
    };
} DataGen;


internal DataGen* data_gen_create_constant_current(State* state,
                                                   f32 value,
                                                   u32 length,
                                                   u32 sample_duration);
internal DataGen* data_gen_create_random_spikes(State* state,
                                                f32 chance,
                                                u32 length,
                                                u32 sample_duration);

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
    };
} DataSample;


internal DataSample* data_gen_sample_create(MemoryArena* arena, DataGen* data, u32 idx);

// NOTE: Currently the easiest thing to do is to give the network when creating inputs
// NOTE: Probably in the future this is not enough but we will see
internal NetworkInputs* data_network_inputs_create(MemoryArena* arena,
                                                   DataSample* sample,
                                                   Network* network,
                                                   u32 time);


#endif //DATA_GEN_H
