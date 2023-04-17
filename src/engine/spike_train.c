internal const char*
spike_train_type_get_c_str(SpikeTrainType type) {
    if (type == SPIKE_TRAIN_ENCODING_IMAGE)
        return "SPIKE_TRAIN_ENCODING_IMAGE";
    if (type == SPIKE_TRAIN_OUTPUT)
        return "SPIKE_TRAIN_OUTPUT";
    return "SPIKE_TRAIN_INVALID";
}


internal SpikeTrain *
spike_train_read(Memory *memory, String *path)
{
    check(memory != NULL, "memory is NULL");
    check(path != NULL, "path is NULL");

    const char* c_path = string_get_c_str(path);
    FILE *fp = fopen(c_path, "rb");
    check(fp != NULL, "couldn't open file %s", c_path);

    // READ HEADER
    u32 time_max = 0;
    u32 n_neurons = 0;
    fread(&time_max, sizeof(u32), 1, fp);
    fread(&n_neurons, sizeof(u32), 1, fp);
    check(time_max > 0, "time_max is 0");
    check(n_neurons > 0, "n_neurons is 0");

    SpikeTrain *spikes = (SpikeTrain *)memory_push_zero(memory, sizeof(SpikeTrain) + sizeof(b32) * time_max * n_neurons);
    check_memory(spikes);
    spikes->time_max = time_max;
    spikes->n_neurons = n_neurons;
    spikes->data = (b32 *)(spikes + 1);

    u32 type = 0;
    fread(&type, sizeof(u32), 1, fp);
    if (type == SPIKE_TRAIN_ENCODING_IMAGE) {
        fread(&spikes->encoding_image.height, sizeof(u32), 1, fp);
        fread(&spikes->encoding_image.width, sizeof(u32), 1, fp);
        check(spikes->encoding_image.height > 0, "height is 0");
        check(spikes->encoding_image.width > 0, "width is 0");
    } else if (type == SPIKE_TRAIN_OUTPUT) {

    } else {
        log_error("Unkown SPIKE TRAIN TYPE %u, %s", type, spike_train_type_get_c_str(type));
    }

    // READ SPIKES
    u32 n_spikes = 0;
    fread(&n_spikes, sizeof(u32), 1, fp);
    NeuronTimeSpike* pairs = (NeuronTimeSpike*)memory_push(memory, sizeof(NeuronTimeSpike) * n_spikes);
    check_memory(pairs);
    fread(pairs, sizeof(NeuronTimeSpike), n_spikes, fp);
    fclose(fp);

    for (u32 i = 0; i < n_spikes; ++i) {
        u32 neuron_i = pairs[i].neuron_i;
        u32 time = pairs[i].time;
        spikes->data[time * spikes->n_neurons + neuron_i] = TRUE;
    }

    return spikes;

    error:
    return NULL;
}

internal b32*
spike_train_get_for_time(SpikeTrain *spikes, u32 time) {
    check(spikes != NULL, "spikes is NULL");
    check(time < spikes->time_max, "time >= spikes->time_max");

    b32* result = spikes->data + time * spikes->n_neurons;
    return result;

    error:
    return NULL;
}
