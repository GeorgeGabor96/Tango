#include "simulator/data_gen.h"


/***********************
* DATA GENERATOR
***********************/
internal DataGen*
data_gen_create_constant_current(f32 value, u32 length, u32 sample_duration) {
    check(length > 0, "length is 0");
    check(sample_duration > 0, "sample_duration is 0");
    
    DataGen* data = (DataGen*) memory_malloc(sizeof(*data),
                                             "data_gen_create_constant_current");
    check_memory(data);
    
    data->type = DATA_GEN_CONSTANT_CURRENT;
    data->length = length;
    data->gen_const_current.value = value;
    data->gen_const_current.duration = sample_duration;
    
    return data;
    
    error:
    return NULL;
}


internal DataGen*
data_gen_create_random_spikes(f32 chance, u32 length, u32 sample_duration) {
    check(length > 0, "length is 0");
    check(sample_duration > 0, "sample_duration is 0");
    check(chance >= 0.0f, "chance should be at least 0, its %f", chance);
    check(chance <= 1.0f, "chance should be at most 1, its %f", chance);
    
    DataGen* data = (DataGen*) memory_malloc(sizeof(*data),
                                             "data_gen_create_random_spikes");
    check_memory(data);
    
    data->type = DATA_GEN_RANDOM_SPIKES;
    data->length = length;
    data->gen_random_spikes.chance = chance;
    data->gen_random_spikes.duration = sample_duration;
    
    return data;
    
    error:
    return NULL;
}


internal void
data_gen_destroy(DataGen* data) {
    check(data != NULL, "data is NULL");
    
    if (data->type == DATA_GEN_CONSTANT_CURRENT) {
        // NOTHING TO DO
    } else if (data->type == DATA_GEN_RANDOM_SPIKES) {
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
    
    DataSample* sample = (DataSample*) memory_malloc(sizeof(*sample),
                                                     "data_gen_sample_create");
    check_memory(sample);
    
    if (data->type == DATA_GEN_CONSTANT_CURRENT) {
        sample->type = DATA_SAMPLE_CONSTANT_CURRENT;
        sample->duration = data->gen_const_current.duration;
        sample->sample_const_current.value = data->gen_const_current.value;
    } else if (data->type == DATA_GEN_RANDOM_SPIKES) {
        sample->type = DATA_SAMPLE_RANDOM_SPIKES;
        sample->duration = data->gen_random_spikes.duration;
        sample->sample_random_spikes.chance = data->gen_random_spikes.chance;
    } else {
        log_error("Unknown Generator type %u", data->type);
    }
    return sample;
    
    error:
    return NULL;
}


internal void
data_gen_sample_destroy(DataSample* sample) {
    check(sample != NULL, "sample is NULL");
    
    memset(sample, 0, sizeof(*sample));
    memory_free(sample);
    
    error:
    return;
}


/***********************
* NETWORK INPUTS
***********************/
internal NetworkInputs*
data_network_inputs_create(DataSample* sample, Network* network, u32 time) {
    check(sample != NULL, "sample is NULL");
    check(network != NULL, "network is NULL");
    
    u32 i = 0;
    u32 j = 0;
    NetworkInput* input = NULL;
    NetworkInputs* inputs = (NetworkInputs*) memory_malloc(sizeof(*inputs),
                                                           "data_gen_inputs_create inputs");
    check_memory(inputs);
    inputs->n_inputs = network->n_in_layers;
    inputs->inputs = (NetworkInput*) memory_malloc(sizeof(*input) * inputs->n_inputs,
                                                   "data_gen_inputs_create inputs->inputs");
    check_memory(inputs->inputs);
    
    if (sample->type == DATA_SAMPLE_RANDOM_SPIKES) {
        for (i = 0; i < inputs->n_inputs; ++i) {
            input = inputs->inputs + i;
            input->type = NETWORK_INPUT_SPIKES;
            input->data = (bool*) memory_calloc(sample->duration, sizeof(bool),
                                                "data_gen_inputs_create RANDOM SPIKES");
            check_memory(input->data);
            // TODO: need a random number generator, don't use the one from C std
            input->n_values = sample->duration; 
        }
    } else if (sample->type == DATA_SAMPLE_CONSTANT_CURRENT) {
        f32* currents = NULL;
        for (i = 0; i < inputs->n_inputs; ++i) {
            input = inputs->inputs + i;
            
            currents = (f32*)memory_malloc(sizeof(f32) * sample->duration,
                                           "data_gen_inputs_create CONSTANT CURRENT");
            check_memory(currents);
            for (j = 0; j < sample->duration; ++j)
                currents[j] = sample->sample_const_current.value;
            
            input->type = NETWORK_INPUT_CURRENT;
            input->data = currents;
            input->n_values = sample->duration;
        }
    } else {
        log_error("Unknown data sample type %u", sample->type);
    }
    
    return inputs;
    
    error:
    // NOTE: Don't free anything yet, when its stable we will free all the things
    return NULL;
}
