#include "simulator/data_gen.h"


/***********************
* DATA GENERATOR
***********************/
internal DataGen*
data_gen_create_constant_current(State* state, f32 value, u32 length, u32 sample_duration) {
    check(state != NULL, "state is NULL");
    check(length > 0, "length is 0");
    check(sample_duration > 0, "sample_duration is 0");
    
    DataGen* data = (DataGen*) memory_arena_push(state->permanent_storage, sizeof(*data));
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
data_gen_create_random_spikes(State* state, f32 chance, u32 length, u32 sample_duration) {
    check(state != NULL, "state is NULL");
    check(length > 0, "length is 0");
    check(sample_duration > 0, "sample_duration is 0");
    check(chance >= 0.0f, "chance should be at least 0, its %f", chance);
    check(chance <= 1.0f, "chance should be at most 1, its %f", chance);
    
    DataGen* data = (DataGen*) memory_arena_push(state->permanent_storage, sizeof(*data));
    check_memory(data);
    
    data->type = DATA_GEN_RANDOM_SPIKES;
    data->length = length;
    data->gen_random_spikes.chance = chance;
    data->gen_random_spikes.duration = sample_duration;
    
    return data;
    
    error:
    return NULL;
}


/***********************
* DATA SAMPLE
***********************/
internal DataSample*
data_gen_sample_create(State* state, DataGen* data, u32 idx) {
    check(state != NULL, "state is NULL");
    check(data != NULL, "data is NULL");
    
    DataSample* sample = (DataSample*) memory_arena_push(state->transient_storage, sizeof(*sample));
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


/***********************
* NETWORK INPUTS
***********************/
internal NetworkInputs*
data_network_inputs_create(State* state, DataSample* sample, Network* network, u32 time) {
    check(state != NULL, "state is NULL");
    check(sample != NULL, "sample is NULL");
    check(network != NULL, "network is NULL");
    
    u32 i = 0;
    u32 j = 0;
    Layer* layer = NULL;
    NetworkInput* input = NULL;
    NetworkInputs* inputs = (NetworkInputs*)
        memory_arena_push(state->transient_storage, sizeof(*inputs));
    check_memory(inputs);
    inputs->n_inputs = network->n_in_layers;
    inputs->inputs = (NetworkInput*) memory_arena_push(state->transient_storage, 
                                                       sizeof(*input) * inputs->n_inputs);
    check_memory(inputs->inputs);
    
    if (sample->type == DATA_SAMPLE_RANDOM_SPIKES) {
        bool* spikes = NULL;
        for (i = 0; i < inputs->n_inputs; ++i) {
            input = inputs->inputs + i;
            layer = network->in_layers[i];
            
            spikes = (bool*)
                memory_arena_push(state->transient_storage, layer->n_neurons * sizeof(bool));
            check_memory(spikes);
            for (j = 0; j < layer->n_neurons; ++j) {
                spikes[j] = random_get_bool(sample->sample_random_spikes.chance);
            }
            input->type = NETWORK_INPUT_SPIKES;
            input->data = spikes;
            input->n_neurons = layer->n_neurons; 
        }
    } else if (sample->type == DATA_SAMPLE_CONSTANT_CURRENT) {
        f32* currents = NULL;
        for (i = 0; i < network->n_in_layers; ++i) {
            input = inputs->inputs + i;
            layer = network->in_layers[i];
            
            currents = (f32*) 
                memory_arena_push(state->transient_storage, sizeof(f32) * layer->n_neurons);
            check_memory(currents);
            for (j = 0; j < layer->n_neurons; ++j) {
                currents[j] = sample->sample_const_current.value;
            }
            input->type = NETWORK_INPUT_CURRENT;
            input->data = currents;
            input->n_neurons = layer->n_neurons;
            
        }
    } else {
        log_error("Unknown data sample type %u", sample->type);
    }
    
    return inputs;
    
    error:
    // NOTE: Don't free anything yet, when its stable we will free all the things
    return NULL;
}
