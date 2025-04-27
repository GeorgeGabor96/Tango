internal Callback*
callback_spikes_dumper_create(Memory* memory, String* output_folder, Network* network) {
    check(memory != NULL, "memory is NULL");
    check(output_folder != NULL, "output_folder is NULL");
    check(network != NULL, "network is NULL");

    b32 result = os_folder_create_str(output_folder);
    check(result == TRUE, "couldn't create folder %s",
          string_get_c_str(output_folder));
    output_folder = string_path_join_c_str(memory, output_folder, "spikes");
    check_memory(output_folder);
    result = os_folder_create_str(output_folder);
    check(result == TRUE, "couldn't create folder %s",
          string_get_c_str(output_folder));

    Callback* callback = (Callback*) memory_push(
        memory, sizeof(*callback));
    check_memory(callback);

    // NOTE: allocate the max number of spike data it can be
    NeuronTimeSpike* spikes_data = (NeuronTimeSpike*)memory_push(
        memory, sizeof(*spikes_data) * network->n_neurons);
    check_memory(spikes_data);

    callback->type = CALLBACK_SPIKES_DUMPER;
    callback->network = network;
    callback->dumper_spikes.output_folder = output_folder;
    callback->dumper_spikes.sample_fp = NULL;
    callback->dumper_spikes.spikes_data = spikes_data;
    callback->dumper_spikes.n_spikes = 0;

    return callback;
    error:
    return NULL;
}


internal CALLBACK_BEGIN_SAMPLE(callback_spikes_dumper_begin_sample)
{
    DumperSpikes* spikes = &callback->dumper_spikes;

    char file_name[100];
    sprintf(file_name, "spikes_%s_e%u.bin", string_get_c_str(sample->name), epoch_i);
    String* file_path = string_path_join_c_str(
        memory, spikes->output_folder, file_name);
    check_memory(file_path);

    FILE* fp = fopen(string_get_c_str(file_path), "wb");
    check(fp != NULL,
          "Couldn't open file %s", string_get_c_str(file_path));

    spikes->n_spikes = 0;
    spikes->sample_fp = fp;

    // NOTE: write a dummy 0 for the number of spikes, this will be overwritten
    // when the sample ends
    fwrite(&spikes->n_spikes,
           sizeof(u32),
           1,
           spikes->sample_fp);

    error:
    return;
}


internal CALLBACK_UPDATE(callback_spikes_dumper_update)
{
    DumperSpikes* spikes = &callback->dumper_spikes;

    // NOTE: get the spike data
    u32 neuron_i = 0;
    u32 spike_i = 0;
    Network* network = callback->network;
    Neuron* neurons = network->neurons;
    Neuron* neuron = NULL;
    NeuronTimeSpike* spikes_data = spikes->spikes_data;
    NeuronTimeSpike* spike_data = NULL;

    for (u32 neuron_i = 0; neuron_i < network->n_neurons; ++neuron_i) {
        neuron = neurons + neuron_i;
        if (neuron->spike == FALSE) continue;

        spike_data = spikes_data + spike_i;
        spike_data->neuron_i = neuron_i;
        spike_data->time = time;

        ++spike_i;
    }

    // NOTE: write the spike data
    fwrite(spikes_data,
           sizeof(NeuronTimeSpike),
           spike_i,
           spikes->sample_fp);

    spikes->n_spikes += spike_i;
}


internal CALLBACK_END_SAMPLE(callback_spikes_dumper_end_sample)
{
    DumperSpikes* spikes = &callback->dumper_spikes;
    Network* network = callback->network;

    // NOTE: Write the number of spikes at the beginning of the file
    fseek(spikes->sample_fp, 0, SEEK_SET);
    fwrite(&spikes->n_spikes, sizeof(u32), 1, spikes->sample_fp);

    fflush(spikes->sample_fp);
    fclose(spikes->sample_fp);

#ifdef _DEBUG_
    u32 n_weights_zero = 0;
    for (u32 synapse_i = 0; synapse_i < network->n_synapses; ++synapse_i) {
        Synapse* synapse = network->synapses + synapse_i;
        if (math_float_equals_f32(synapse->weight, 0.0f)) n_weights_zero++;
    }
    log_info("N_weights zero %u", n_weights_zero);
#endif
}


internal CALLBACK_BEGIN_EPOCH(callback_spikes_dumper_begin_epoch)
{

}


internal CALLBACK_END_EPOCH(callback_spikes_dumper_end_epoch)
{

}