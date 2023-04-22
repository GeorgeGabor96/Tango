internal Callback*
callback_weights_dumper_create(Memory* memory, String* output_folder, Network* network) {
    check(memory != NULL, "memory is NULL");
    check(output_folder != NULL, "output_folder is NULL");
    check(network != NULL, "network is NULL");

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
    callback->dumper_weights.network = network;
    callback->dumper_weights.output_folder = output_folder;
    callback->dumper_weights.sample_fp = NULL;
    callback->dumper_weights.weights = weights;

    return callback;

    error:
    return NULL;
}


internal void
callback_weights_dumper_begin_sample(Callback* callback, DataSample* sample, Memory* memory) {
    DumperWeights* data = &callback->dumper_weights;

    char file_name[100];
    sprintf(file_name, "weights_%s.bin", string_get_c_str(sample->name));
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

    error:
    return;
}


internal void
callback_weights_dumper_update(Callback* callback, Memory* memory) {
    DumperWeights* data = &callback->dumper_weights;

    u32 synapse_i = 0;
    Network* network = data->network;
    Synapse* synapse = NULL;
    Synapse* synapses = network->synapses;
    float* weights = data->weights;

    for (synapse_i = 0; synapse_i < network->n_synapses; ++synapse_i) {
        synapse = synapses + synapse_i;
        weights[synapse_i] = synapse->weight;
    }

    fwrite(weights, sizeof(float), network->n_synapses, data->sample_fp);
}


internal void
callback_weights_dumper_end_sample(Callback* callback, Memory* memory) {
    DumperWeights* data = &callback->dumper_weights;

    fflush(data->sample_fp);
    fclose(data->sample_fp);
}
