/* date = October 13th 2022 8:19 pm */

#ifndef DATA_GEN_H
#define DATA_GEN_H


#include "common.h"


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
        } const_current;
        struct {
            f32 chance;
        } random_spikes;
    }
} DataGen;


typedef enum {
    DATA_SAMPLE_INVALID,
    DATA_SAMPLE_SPIKES,
    DATA_SAMPLE_VOLTAGES,
} DataSampleType;


typedef struct DataSample {
    DataSampleType type;
    u32 duration;
    
} Sample;


internal void data_gen_create_constant_current(f32 value);
internal void data_gen_create_random_spikes(f32 chance);

internal void data_gen_destroy(DataGen* data);

internal void* data_gen_get_sample(DataGen* data, u32 idx);


#endif //DATA_GEN_H
