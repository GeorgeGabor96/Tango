/***********************
* DATA GENERATOR
***********************/
internal DataGen*
data_gen_create_constant_current(Memory* memory, f32 value, u32 n_samples, u32 sample_duration) {
    check(memory != NULL, "memory is NULL");
    check(n_samples > 0, "n_samples is 0");
    check(sample_duration > 0, "sample_duration is 0");

    DataGen* data = (DataGen*)memory_push(memory, sizeof(*data));
    check_memory(data);

    data->type = DATA_GEN_CONSTANT_CURRENT;
    data->n_samples = n_samples;
    data->sample_i = 0;
    data->sample_duration = sample_duration;
    data->const_current.value = value;

    return data;

    error:
    return NULL;
}


internal DataGen*
data_gen_create_random_spikes(Memory* memory, Random* random, f32 chance, u32 n_samples, u32 sample_duration) {
    check(memory != NULL, "memory is NULL");
    check(random != NULL, "random is NULL");
    check(n_samples > 0, "n_samples is 0");
    check(sample_duration > 0, "sample_duration is 0");
    check(chance >= 0.0f, "chance should be at least 0, its %f", chance);
    check(chance <= 1.0f, "chance should be at most 1, its %f", chance);

    DataGen* data = (DataGen*) memory_push(memory, sizeof(*data));
    check_memory(data);

    data->type = DATA_GEN_RANDOM_SPIKES;
    data->n_samples = n_samples;
    data->sample_i = 0;
    data->sample_duration = sample_duration;
    data->random_spikes.random = random;
    data->random_spikes.chance = chance;

    return data;

    error:
    return NULL;
}


internal DataGen*
data_gen_create_spike_pulses(Memory* memory,
                             Random* random,
                             u32 n_samples,
                             u32 sample_duration,
                             u32 first_pulse_time,
                             u32 pulse_duration,
                             u32 between_pulses_duration,
                             f32 pulse_spike_chance,
                             f32 between_pulses_spike_chance) {
    check(memory != NULL, "memory is NULL");
    check(random != NULL, "random is NULL");
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

    DataGen* data = (DataGen*) memory_push(memory, sizeof(*data));
    check_memory(data);

    data->type = DATA_GEN_SPIKE_PULSES;
    data->n_samples = n_samples;
    data->sample_i = 0;
    data->sample_duration = sample_duration;

    DataGenSpikePulses* pulses = &(data->spike_pulses);
    pulses->random = random;
    pulses->first_pulse_time = first_pulse_time;
    pulses->pulse_duration = pulse_duration;
    pulses->between_pulses_duration = between_pulses_duration;
    pulses->pulse_spike_chance = pulse_spike_chance;
    pulses->between_pulses_spike_chance = between_pulses_spike_chance;

    return data;

    error:
    return NULL;
}


internal DataGen*
data_gen_create_spike_train(Memory* memory,
                            u32 duration,
                            const char* encodings_path,
                            const char* listing_file,
                            u32 max_time_to_use_from_train,
                            u32 n_samples) {
    check(memory != NULL, "memory is NULL");
    check(duration > 0, "duration is 0");
    check(encodings_path != NULL, "encodings_path is NULL");
    check(listing_file != NULL, "listing_file is NULL");

    // NOTE: if its 0 set it to -1 so that it doesn't effect
    if (max_time_to_use_from_train == 0) max_time_to_use_from_train = (u32)-1;

    DataGen* data = (DataGen*)memory_push(memory, sizeof(*data));
    check_memory(data);

    // NOTE: read the samples
    FILE* fp = fopen(listing_file, "r");
    check(fp != NULL, "couldn't open listing file %s", listing_file);
    char buffer[128] = { 0 }; // Names should not be this long
    u32 n_samples_in_file = 0;
    StringNode* sample_name_list = NULL;

    while (!feof(fp)) {
        fscanf(fp, "%s\n", buffer);
        String* sample_name = string_create(memory, buffer);
        check_memory(sample_name);

        StringNode* node = (StringNode*)memory_push(memory, sizeof(*node));
        check_memory(node);

        node->name = sample_name;
        node->next = NULL;

        if (sample_name_list == NULL) {
            sample_name_list = node;
        } else {
            node->next = sample_name_list;
            sample_name_list = node;
        }
        ++n_samples_in_file;
        memset(buffer, 0, sizeof(buffer));
    }

    String* encodings_path_str = string_create(memory, encodings_path);
    check_memory(encodings_path_str);

    data->type = DATA_GEN_SPIKE_TRAIN;
    data->sample_duration = duration;
    if (n_samples == 0)
        data->n_samples = n_samples_in_file;
    else
        data->n_samples = n_samples;
    data->sample_i = 0;

    data->spike_train.first_file_name = sample_name_list;
    data->spike_train.current_sample = sample_name_list;
    data->spike_train.encodings_path = encodings_path_str;
    data->spike_train.max_time_to_use_from_train = max_time_to_use_from_train;

    return data;

    error:
    return NULL;
}

internal DataGen*
data_gen_create_basic_experiment(Memory* memory, Random* random, f32 spike_chance, u32 n_samples)
{
    check(memory != NULL, "memory is NULL");
    check(spike_chance >= 0.0f, "spike chance should be >= 0");
    check(spike_chance <= 1.0f, "spike change should be <= 1");

    DataGen* data = (DataGen*) memory_push(memory, sizeof(*data));
    check_memory(data);

    data->type = DATA_GEN_BASIC_EXPERIMENT;
    data->n_samples = n_samples;
    data->sample_i = 0;
    data->sample_duration = 1000;
    data->basic_exp.random = random;
    data->basic_exp.spike_chance = spike_chance;
    return data;

    error:
    return NULL;
}



/***********************
* DATA SAMPLE
***********************/
internal DataSample*
data_gen_sample_create(Memory* memory, DataGen* data, u32 idx) {
    check(memory != NULL, "memory is NULL");
    check(data != NULL, "data is NULL");

    DataSample* sample = (DataSample*) memory_push(memory, sizeof(*sample));
    check_memory(sample);

    sample->duration = data->sample_duration;
    sample->data_gen = data;
    sample->sample_i = data->sample_i;

    char sample_name[100];
    if (data->type == DATA_GEN_CONSTANT_CURRENT) {
        sample->type = DATA_SAMPLE_CONSTANT_CURRENT;
        sprintf(sample_name, "constant_current_%06d", sample->sample_i);
    } else if (data->type == DATA_GEN_RANDOM_SPIKES) {
        sample->type = DATA_SAMPLE_RANDOM_SPIKES;
        sprintf(sample_name, "random_spikes_%06d", sample->sample_i);
    } else if (data->type == DATA_GEN_SPIKE_PULSES) {
        sample->type = DATA_SAMPLE_SPIKE_PULSES;
        sprintf(sample_name, "spike_pulses_%06d", sample->sample_i);

        DataGenSpikePulses* data_pulses = &(data->spike_pulses);
        DataSampleSpikePulses* sample_pulses = &(sample->spike_pulses);

        if (data_pulses->first_pulse_time == 0) {
            sample_pulses->in_pulse = TRUE;
        } else {
            sample_pulses->in_pulse = FALSE;
        }
        sample_pulses->next_pulse_time = data_pulses->first_pulse_time;
        sample_pulses->next_between_pulses_time = data_pulses->first_pulse_time + data_pulses->pulse_duration;
    } else if (data->type == DATA_GEN_SPIKE_TRAIN) {
        sample->type = DATA_SAMPLE_SPIKE_TRAIN;

        DataGenSpikeTrain* data_spike_train = &(data->spike_train);
        DataSampleSpikeTrain* sample_spike_train = &(sample->spike_train);

        if (data_spike_train->current_sample == NULL) {
            if (data->sample_i < data->n_samples) {
                data_spike_train->current_sample = data_spike_train->first_file_name;
            } else {
                log_error("NO MORE SAMPLES. Shouldn't be called this much");
            }
        }
        sprintf(sample_name, "%s.bin", string_get_c_str(data_spike_train->current_sample->name));
        String* sample_path = string_path_join_c_str(memory, data_spike_train->encodings_path, sample_name);
        check_memory(sample_path);

        SpikeTrain* spikes = spike_train_read(memory, sample_path);
        check_memory(spikes);
        sample_spike_train->spikes = spikes;

        sprintf(sample_name, "s%06d_%s",
            sample->sample_i,
            string_get_c_str(data_spike_train->current_sample->name));
        data_spike_train->current_sample = data_spike_train->current_sample->next;

    } else if (data->type == DATA_GEN_BASIC_EXPERIMENT) {
        sample->type = DATA_SAMPLE_BASIC_EXPERIMENT;
        sprintf(sample_name, "basic_exp_%06d", sample->sample_i);
        sample->basic_exp.spike_chance = data->basic_exp.spike_chance;

    } else {
        log_error("Unknown Generator type %u", data->type);
    }
    sample->name = string_create(memory, sample_name);
    check_memory(sample->name);

    ++(data->sample_i);
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

    b32* spikes = NULL;
    f32* currents = NULL;

    NetworkLayerLink* it = NULL;

    for (i = 0, it = network->in_layers.first; i < inputs->n_inputs && it != NULL; ++i, it = it->next) {
        input = inputs->inputs + i;
        layer = it->layer;

        if (sample->type == DATA_SAMPLE_RANDOM_SPIKES) {
            DataGenRandomSpikes* random_spikes = &sample->data_gen->random_spikes;
            spikes = (b32*)memory_push(memory, layer->n_neurons * sizeof(b32));
            check_memory(spikes);
            for (j = 0; j < layer->n_neurons; ++j)
                spikes[j] = random_get_b8(random_spikes->random, random_spikes->chance);

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
            spikes = (b32*) memory_push(memory, layer->n_neurons * sizeof(b32));
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
                    spikes[j] = random_get_b32(data_pulses->random, data_pulses->pulse_spike_chance);
                } else {
                    spikes[j] = random_get_b32(data_pulses->random, data_pulses->between_pulses_spike_chance);
                }
            }

            input->type = INPUT_SPIKES;
            input->spikes.spikes = spikes;
            input->spikes.n_spikes = layer->n_neurons;
        } else if (sample->type == DATA_SAMPLE_SPIKE_TRAIN) {
            check(network->n_in_layers == 1,
                  "For spike train data generation there needs to be exactly one input layer");

            DataSampleSpikeTrain* spike_train = &(sample->spike_train);
            check(layer->n_neurons == spike_train->spikes->n_neurons,
                  "For spike train data generation the number of neurons in the train and in the input layer should be the same");

            input->type = INPUT_SPIKES;
            // NOTE: If the spike_train time is passed or the desired max time from which to use the train is passed, set 0's
            if (time >= spike_train->spikes->time_max || time >= sample->data_gen->spike_train.max_time_to_use_from_train) {
                spikes = (b32*)memory_push_zero(memory, layer->n_neurons * sizeof(b32));
                check_memory(spikes);
            } else {
                spikes = spike_train_get_for_time(spike_train->spikes, time);
            }
            input->spikes.spikes = spikes;
            input->spikes.n_spikes = layer->n_neurons;
        } else if (sample->type == DATA_SAMPLE_BASIC_EXPERIMENT) {
            check(layer->n_neurons == 4, "Should be 4 input neurons for the basic exp");
            spikes = (b32*) memory_push(memory, layer->n_neurons * sizeof(b32));
            check_memory(spikes);

            Random* random = sample->data_gen->basic_exp.random;
            f32 spike_chance = sample->basic_exp.spike_chance;
            u32 idx = sample->sample_i % 4;
            if (idx == 0) // NO INPUT
            {
                spikes[0] = FALSE;
                spikes[1] = FALSE;
                spikes[2] = FALSE;
                spikes[3] = FALSE;
            }
            else if (idx == 1) // 0 and 2 spiked with spike chance
            {
                spikes[0] = random_get_b32(random, spike_chance);
                spikes[1] = FALSE;
                spikes[2] = random_get_b32(random, spike_chance);
                spikes[3] = FALSE;
            }
            else if (idx == 2) // 1 and 3 spiked with spike chance
            {
                spikes[0] = FALSE;
                spikes[1] = random_get_b32(random, spike_chance);
                spikes[2] = FALSE;
                spikes[3] = random_get_b32(random, spike_chance);
            }
            else if (idx == 3) // all spiked with spike chance
            {
                spikes[0] = random_get_b32(random, spike_chance);
                spikes[1] = random_get_b32(random, spike_chance);
                spikes[2] = random_get_b32(random, spike_chance);
                spikes[3] = random_get_b32(random, spike_chance);
            }

            input->type = INPUT_SPIKES;
            input->spikes.spikes = spikes;
            input->spikes.n_spikes = layer->n_neurons;

        } else {
            log_error("Unknown data sample type %u", sample->type);
        }
    }
    return inputs;

    error:
    return NULL;
}
