#include "simulator/data_gen.h"


/***********************
* DATA GENERATOR
***********************/
internal DataGen*
data_gen_create_constant_current(State* state, f32 value, u32 n_samples, u32 sample_duration) {
    check(state != NULL, "state is NULL");
    check(n_samples > 0, "n_samples is 0");
    check(sample_duration > 0, "sample_duration is 0");
    
    DataGen* data = (DataGen*) memory_arena_push(state->permanent_storage, sizeof(*data));
    check_memory(data);
    
    data->type = DATA_GEN_CONSTANT_CURRENT;
    data->n_samples = n_samples;
    data->sample_duration = sample_duration;
    data->gen_const_current.value = value;
    
    return data;
    
    error:
    return NULL;
}


internal DataGen*
data_gen_create_random_spikes(State* state, f32 chance, u32 n_samples, u32 sample_duration) {
    check(state != NULL, "state is NULL");
    check(n_samples > 0, "n_samples is 0");
    check(sample_duration > 0, "sample_duration is 0");
    check(chance >= 0.0f, "chance should be at least 0, its %f", chance);
    check(chance <= 1.0f, "chance should be at most 1, its %f", chance);
    
    DataGen* data = (DataGen*) memory_arena_push(state->permanent_storage, sizeof(*data));
    check_memory(data);
    
    data->type = DATA_GEN_RANDOM_SPIKES;
    data->n_samples = n_samples;
    data->sample_duration = sample_duration;
    data->gen_random_spikes.chance = chance;
    
    return data;
    
    error:
    return NULL;
}

internal DataGen*
data_gen_create_spike_pulses(State* state,
                             u32 n_samples,
                             u32 sample_duration,
                             u32 first_pulse_time,
                             u32 pulse_duration,
                             u32 between_pulses_duration,
                             f32 pulse_spike_chance,
                             f32 between_pulses_spike_chance) {
    check(state != NULL, "state is NULL");
    check(n_samples > 0, "n_samples is 0");
    check(sample_duration > 0, "sample_duration is 0");
    check(pulse_duration > 0, "pulse_duration is 0");
    check(between_pulses_duration > 0, "between_pulses_duration is 0");
    check(pulse_spike_chance >= 0.0f && pulse_spike_chance <= 1.0f,
          "pulse_spike_chance is %f not in [0, 1]",
          pulse_spike_chance);
    check(between_pulses_spike_chance >= 0.0f &&
          between_pulses_spike_chance <= 1.0f,
          "between_pulses_spike_chance is %f not in [0, 1]",
          between_pulses_spike_chance);
    
    DataGen* data = (DataGen*) memory_arena_push(state->permanent_storage, sizeof(*data));
    check_memory(data);
    
    data->type = DATA_GEN_SPIKE_PULSES;
    data->n_samples = n_samples;
    data->sample_duration = sample_duration;
    data->gen_spike_pulses.first_pulse_time = first_pulse_time;
    data->gen_spike_pulses.pulse_duration = pulse_duration;
    data->gen_spike_pulses.between_pulses_duration = between_pulses_duration;
    data->gen_spike_pulses.pulse_spike_chance = pulse_spike_chance;
    data->gen_spike_pulses.between_pulses_spike_chance = between_pulses_spike_chance;
    
    return data;
    
    error:
    return NULL;
}



/***********************
* DATA SAMPLE
***********************/
internal DataSample*
data_gen_sample_create(MemoryArena* arena, DataGen* data, u32 idx) {
    check(arena != NULL, "arena is NULL");
    check(data != NULL, "data is NULL");
    
    DataSample* sample = (DataSample*) memory_arena_push(arena, sizeof(*sample));
    check_memory(sample);
    
    if (data->type == DATA_GEN_CONSTANT_CURRENT) {
        sample->type = DATA_SAMPLE_CONSTANT_CURRENT;
        sample->duration = data->sample_duration;
        sample->sample_const_current.value = data->gen_const_current.value;
    } else if (data->type == DATA_GEN_RANDOM_SPIKES) {
        sample->type = DATA_SAMPLE_RANDOM_SPIKES;
        sample->duration = data->sample_duration;
        sample->sample_random_spikes.chance = data->gen_random_spikes.chance;
    } else if (data->type == DATA_GEN_SPIKE_PULSES) {
        sample->type = DATA_SAMPLE_SPIKE_PULSES;
        sample->duration = data->sample_duration;
        
        if (data->gen_spike_pulses.first_pulse_time == 0) {
            sample->sample_spike_pulses.in_pulse = TRUE;
        } else {
            sample->sample_spike_pulses.in_pulse = FALSE;
        }
        sample->sample_spike_pulses.next_pulse_time = data->gen_spike_pulses.first_pulse_time;
        sample->sample_spike_pulses.next_between_pulses_time = data->gen_spike_pulses.first_pulse_time + 
            data->gen_spike_pulses.pulse_duration;
        sample->sample_spike_pulses.pulse_duration = data->gen_spike_pulses.pulse_duration;
        sample->sample_spike_pulses.between_pulses_duration = data->gen_spike_pulses.between_pulses_duration;
        sample->sample_spike_pulses.pulse_spike_chance = data->gen_spike_pulses.pulse_spike_chance;
        sample->sample_spike_pulses.between_pulses_spike_chance = data->gen_spike_pulses.between_pulses_spike_chance;
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
data_network_inputs_create(MemoryArena* arena, DataSample* sample,
                           Network* network, u32 time) {
    check(arena != NULL, "arena is NULL");
    check(sample != NULL, "sample is NULL");
    check(network != NULL, "network is NULL");
    
    u32 i = 0;
    u32 j = 0;
    Layer* layer = NULL;
    NetworkInput* input = NULL;
    NetworkInputs* inputs = (NetworkInputs*)
        memory_arena_push(arena, sizeof(*inputs));
    check_memory(inputs);
    inputs->n_inputs = network->n_in_layers;
    inputs->inputs = (NetworkInput*)
        memory_arena_push(arena, sizeof(*input) * inputs->n_inputs);
    check_memory(inputs->inputs);
    
    if (sample->type == DATA_SAMPLE_RANDOM_SPIKES) {
        bool* spikes = NULL;
        for (i = 0; i < inputs->n_inputs; ++i) {
            input = inputs->inputs + i;
            layer = network->in_layers[i];
            
            spikes = (bool*)
                memory_arena_push(arena, layer->n_neurons * sizeof(bool));
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
                memory_arena_push(arena, sizeof(f32) * layer->n_neurons);
            check_memory(currents);
            for (j = 0; j < layer->n_neurons; ++j) {
                currents[j] = sample->sample_const_current.value;
            }
            input->type = NETWORK_INPUT_CURRENT;
            input->data = currents;
            input->n_neurons = layer->n_neurons;
            
        }
    } else if (sample->type == DATA_SAMPLE_SPIKE_PULSES) {
        bool* spikes = NULL;
        
        for (i = 0; i < inputs->n_inputs; ++i) {
            input = inputs->inputs + i;
            layer = network->in_layers[i];
            
            spikes = (bool*) memory_arena_push(arena, layer->n_neurons * sizeof(bool));
            check_memory(spikes);
            
            for (j = 0; j < layer->n_neurons; ++j) {
                if (time == sample->sample_spike_pulses.next_pulse_time) {
                    sample->sample_spike_pulses.in_pulse = TRUE;
                    sample->sample_spike_pulses.next_pulse_time = time +
                        sample->sample_spike_pulses.pulse_duration + 
                        sample->sample_spike_pulses.between_pulses_duration;
                } else if (time == sample->sample_spike_pulses.next_between_pulses_time) {
                    sample->sample_spike_pulses.in_pulse = FALSE;
                    sample->sample_spike_pulses.next_between_pulses_time = time + 
                        sample->sample_spike_pulses.between_pulses_duration +
                        sample->sample_spike_pulses.pulse_duration;
                }
                
                if (sample->sample_spike_pulses.in_pulse == TRUE) {
                    spikes[j] = random_get_bool(sample->sample_spike_pulses.pulse_spike_chance);
                } else {
                    spikes[j] = 
                        random_get_bool(sample->sample_spike_pulses.between_pulses_spike_chance);
                }
                
            }
            input->type = NETWORK_INPUT_SPIKES;
            input->data = spikes;
            input->n_neurons = layer->n_neurons;
        }
    }else {
        log_error("Unknown data sample type %u", sample->type);
    }
    
    return inputs;
    
    error:
    return NULL;
}
