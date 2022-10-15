#include "simulator/data_gen.h"


/***********************
* DATA GENERATOR
***********************/
internal void
data_gen_create_constant_current(f32 value, u32 length, u32 sample_duration) {
    check(length > 0, "length is 0");
    check(sample_duration > 0, "sample_duration is 0");
    
    DataGen* data = (DataGen*) memory_malloc(sizeof(*data));
    check_memory(data);
    
    data->type = DATA_GEN_CONSTANT_CURRENT;
    data->const_current.value = value;
    data->const_current.sample_duration = sample_duration
        
        return data;
    
    error:
    return NULL;
}


internal void
data_gen_create_random_spikes(f32 chance, u32 length, u32 sample_duration) {
    check(length > 0, "length is 0");
    check(sample_duration > 0, "sample_duration is 0");
    check(chance >= 0.0f, "chance should be at least 0, its %f", chance);
    check(chance <= 1.0f, "chance should be at most 1, its %f", chance);
    
    DataGen* data = (DataGen*) memory_malloc(sizeof(*data));
    check_memory(data);
    
    data->type = DATA_GEN_RANDOM_SPIKES;
    data->random_spikes.chance = chance;
    data->ranodm_spikes.sample_duration = sample_duration;
    
    return data;
    
    error:
    return NULL;
}


internal void
data_gen_destroy(DataGen* data) {
    check(data != NULL, "data is NULL");
    
    if (Data->type == DATA_GEN_CONSTANT_CURRENT) {
        // NOTHING TO DO
    } else if (Data->type == DATA_GEN_RANDOM_SPIKES) {
        // NOTHING TO DO
    }
    memset(data, 0, sizeof(*data));
    memory_free(data);
    
    error:
    return;
}



/***********************
* DATA SAMPLE
***********************/
internal DataSample*
data_gen_sample_create(DataGen* data, u32 idx) {
    check(data != NULL, "data is NULL");
    
    DataSample* sample = (DataSample*) memory_malloc(sizeof(*sample));
    check_memory(sample);
    
    if (data->type == DATA_GEN_CONSTANT_CURRENT) {
        sample->type = DATA_SAMPLE_CONSTANT_CURRENT;
        sample->duration = data->const_current.duration;
    } else if (data->type == DATA_GEN_RANODM_SPIKES) {
        sample->type = DATA_SAMPLE_RANDOM_SPIKES;
        sample->duration = data->random_spikes.duration;
    } else {
        log_error("Unknown Generator type %u", data->type);
    }
    return sample;
    
    error:
    return NULL;
}


internal void
data_gen_sample_destory(DataSample* sample) {
    check(sample != NULL, "sample is NULL");
    
    memset(sample, 0, sizeof(*sample));
    memory_free(sample);
    
    error;
    return;
}
