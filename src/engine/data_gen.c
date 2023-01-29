/***********************
* DATA GENERATOR
***********************/
internal DataGen*
data_gen_create_constant_current(State* state, f32 value, u32 n_samples, u32 sample_duration) {
    check(state != NULL, "state is NULL");
    check(n_samples > 0, "n_samples is 0");
    check(sample_duration > 0, "sample_duration is 0");

    DataGen* data = (DataGen*)memory_push(state->permanent_storage, sizeof(*data));
    check_memory(data);

    data->type = DATA_GEN_CONSTANT_CURRENT;
    data->n_samples = n_samples;
    data->sample_duration = sample_duration;
    data->const_current.value = value;

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

    DataGen* data = (DataGen*) memory_push(state->permanent_storage, sizeof(*data));
    check_memory(data);

    data->type = DATA_GEN_RANDOM_SPIKES;
    data->n_samples = n_samples;
    data->sample_duration = sample_duration;
    data->random_spikes.chance = chance;

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

    DataGen* data = (DataGen*) memory_push(state->permanent_storage, sizeof(*data));
    check_memory(data);

    data->type = DATA_GEN_SPIKE_PULSES;
    data->n_samples = n_samples;
    data->sample_duration = sample_duration;

    DataGenSpikePulses* pulses = &(data->spike_pulses);
    pulses->first_pulse_time = first_pulse_time;
    pulses->pulse_duration = pulse_duration;
    pulses->between_pulses_duration = between_pulses_duration;
    pulses->pulse_spike_chance = pulse_spike_chance;
    pulses->between_pulses_spike_chance = between_pulses_spike_chance;

    return data;

    error:
    return NULL;
}


/***********************
* DATA SAMPLE
***********************/
internal DataSample*
data_gen_sample_create(Memory* memory, DataGen* data, u32 idx) {
    check(memory != NULL, "memor yis NULL");
    check(data != NULL, "data is NULL");

    DataSample* sample = (DataSample*) memory_push(memory, sizeof(*sample));
    check_memory(sample);

    sample->duration = data->sample_duration;
    sample->data_gen = data;

    if (data->type == DATA_GEN_CONSTANT_CURRENT) {
        sample->type = DATA_SAMPLE_CONSTANT_CURRENT;
    } else if (data->type == DATA_GEN_RANDOM_SPIKES) {
        sample->type = DATA_SAMPLE_RANDOM_SPIKES;
    } else if (data->type == DATA_GEN_SPIKE_PULSES) {
        sample->type = DATA_SAMPLE_SPIKE_PULSES;

        DataGenSpikePulses* data_pulses = &(data->spike_pulses);
        DataSampleSpikePulses* sample_pulses = &(sample->spike_pulses);

        if (data_pulses->first_pulse_time == 0) {
            sample_pulses->in_pulse = TRUE;
        } else {
            sample_pulses->in_pulse = FALSE;
        }
        sample_pulses->next_pulse_time = data_pulses->first_pulse_time;
        sample_pulses->next_between_pulses_time = data_pulses->first_pulse_time + data_pulses->pulse_duration;
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
internal Inputs*
data_network_inputs_create(Memory* memory, DataSample* sample, Network* network, u32 time) {
    check(memory != NULL, "memory is NULL");
    check(sample != NULL, "sample is NULL");
    check(network != NULL, "network is NULL");

    u32 i = 0;
    u32 j = 0;
    Layer* layer = NULL;
    Input* input = NULL;
    Inputs* inputs = (Inputs*)memory_push(memory, sizeof(*inputs));
    check_memory(inputs);
    inputs->n_inputs = network->n_in_layers;
    inputs->inputs = (Input*)memory_push(memory, sizeof(*input) * inputs->n_inputs);
    check_memory(inputs->inputs);

    bool* spikes = NULL;
    f32* currents = NULL;

    NetworkLayerLink* it = NULL;

    for (i = 0, it = network->in_layers; i < inputs->n_inputs && it != NULL; ++i, it = it->next) {
        input = inputs->inputs + i;
        layer = it->layer;

        if (sample->type == DATA_SAMPLE_RANDOM_SPIKES) {
            spikes = (bool*)memory_push(memory, layer->n_neurons * sizeof(bool));
            check_memory(spikes);
            for (j = 0; j < layer->n_neurons; ++j)
                spikes[j] = random_get_bool(sample->data_gen->random_spikes.chance);

            input->type = INPUT_SPIKES;
            input->spikes.spikes = spikes;
            input->spikes.n_spikes = layer->n_neurons;
        }
        else if (sample->type == DATA_SAMPLE_CONSTANT_CURRENT) {
            currents = (f32*)memory_push(memory, sizeof(f32) * layer->n_neurons);
            check_memory(currents);
            for (j = 0; j < layer->n_neurons; ++j)
                currents[j] = sample->data_gen->const_current.value;

            input->type = INPUT_CURRENT;
            input->currents.currents = currents;
            input->currents.n_currents = layer->n_neurons;
        }
        else if (sample->type == DATA_SAMPLE_SPIKE_PULSES) {
            spikes = (bool*) memory_push(memory, layer->n_neurons * sizeof(bool));
            check_memory(spikes);

            DataGenSpikePulses* data_pulses = &(sample->data_gen->spike_pulses);
            DataSampleSpikePulses* sample_pulses = &(sample->spike_pulses);

            for (j = 0; j < layer->n_neurons; ++j) {
                if (time == sample_pulses->next_pulse_time) {
                    sample_pulses->in_pulse = TRUE;
                    sample_pulses->next_pulse_time = time + data_pulses->pulse_duration +
                                                        data_pulses->between_pulses_duration;
                } else if (time == sample_pulses->next_between_pulses_time) {
                    sample_pulses->in_pulse = FALSE;
                    sample_pulses->next_between_pulses_time = time +
                            data_pulses->between_pulses_duration + data_pulses->pulse_duration;
                }

                if (sample_pulses->in_pulse == TRUE) {
                    spikes[j] = random_get_bool(data_pulses->pulse_spike_chance);
                } else {
                    spikes[j] = random_get_bool(data_pulses->between_pulses_spike_chance);
                }
            }

            input->type = INPUT_SPIKES;
            input->spikes.spikes = spikes;
            input->spikes.n_spikes = layer->n_neurons;
        }
        else {
            log_error("Unknown data sample type %u", sample->type);
        }
    }
    return inputs;

    error:
    return NULL;
}
