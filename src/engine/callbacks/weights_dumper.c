internal Callback*
callback_weights_dumper_create(
        Memory* memory,
        u32 sample_step,
        u32 time_step,
        String* output_folder,
        Network* network) {
    check(memory != NULL, "memory is NULL");
    check(output_folder != NULL, "output_folder is NULL");
    check(network != NULL, "network is NULL");

    // NOTE: default we dump every sample
    if (sample_step == 0) sample_step = 1;
    if (time_step == 0) time_step = 1;

    b32 result = os_folder_create_str(output_folder);
    check(result == TRUE, "couldn't create folder %s",
          string_get_c_str(output_folder));
    output_folder = string_path_join_c_str(memory, output_folder, "weights");
    check_memory(output_folder);
    result = os_folder_create_str(output_folder);
    check(result == TRUE, "coudn't create folder %s",
          string_get_c_str(output_folder));

    Callback* callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    float* weights = (float*)memory_push(memory, sizeof(float) * network->n_synapses);
    check_memory(weights);

    callback->type = CALLBACK_WEIGHTS_DUMPER;
    callback->network = network;
    callback->dumper_weights.time_step = time_step;
    callback->dumper_weights.next_time_to_dump_i = 0;
    callback->dumper_weights.sample_step = sample_step;
    callback->dumper_weights.next_sample_to_dump_i = 0;
    callback->dumper_weights.output_folder = output_folder;
    callback->dumper_weights.sample_fp = NULL;
    callback->dumper_weights.weights = weights;

    return callback;

    error:
    return NULL;
}


internal CALLBACK_BEGIN_SAMPLE(callback_weights_dumper_begin_sample)
{
    DumperWeights* data = &callback->dumper_weights;

    if (sample->sample_i != data->next_sample_to_dump_i) {
        data->sample_fp = NULL;
        return;
    }

    data->next_sample_to_dump_i += data->sample_step;
    data->next_time_to_dump_i = 0;

    char file_name[100];
    sprintf(file_name, "weights_%s_e%u.bin", string_get_c_str(sample->name), epoch_i);
    String* file_name_s = string_create(memory, file_name);
    check_memory(file_name_s);

    String* file_path = string_path_join(memory,
                                         data->output_folder,
                                         file_name_s);
    check(file_path != NULL, "Couldn't build path for weights sample file %s", file_name);

    data->sample_fp = fopen(string_get_c_str(file_path), "wb");
    check(data->sample_fp != NULL,
          "Couldn't open weights file %s",
          string_get_c_str(file_path));
    // NOTE: write frequency to know when reading the file
    fwrite(&data->time_step, sizeof(u32), 1, data->sample_fp);

    error:
    return;
}


internal CALLBACK_UPDATE(callback_weights_dumper_update)
{
    DumperWeights* data = &callback->dumper_weights;
    if (data->sample_fp == NULL) return;

    if (time != data->next_time_to_dump_i) return;
    data->next_time_to_dump_i += data->time_step;

    u32 synapse_i = 0;
    Network* network = callback->network;
    Synapse* synapse = NULL;
    Synapse* synapses = network->synapses;
    float* weights = data->weights;

    for (synapse_i = 0; synapse_i < network->n_synapses; ++synapse_i) {
        synapse = synapses + synapse_i;
        weights[synapse_i] = synapse->weight;
    }

    fwrite(weights, sizeof(float), network->n_synapses, data->sample_fp);
}


internal CALLBACK_END_SAMPLE(callback_weights_dumper_end_sample)
{
    DumperWeights* data = &callback->dumper_weights;

    if (data->sample_fp == NULL) return;

    fflush(data->sample_fp);
    fclose(data->sample_fp);
}


internal CALLBACK_BEGIN_EPOCH(callback_weights_dumper_begin_epoch)
{
    callback->dumper_weights.next_sample_to_dump_i = 0;
}


internal CALLBACK_END_EPOCH(callback_weights_dumper_end_epoch)
{

}