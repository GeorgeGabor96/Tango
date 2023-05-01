internal Callback*
callback_meta_dumper_create(Memory* memory, String* output_folder, Network* network) {
    check(memory != NULL, "memory is NULL");
    check(output_folder != NULL, "output_folder is NULL");
    check(network != NULL, "network is NULL");

    b32 result = os_folder_create_str(output_folder);
    check(result == TRUE, "couldn't create folder %s",
          string_get_c_str(output_folder));

    // NOTE: Write the network meta information
    String* meta_file = string_path_join_c_str(memory, output_folder, "meta.bin");
    check_memory(meta_file);

    FILE* fp = fopen(string_get_c_str(meta_file), "wb");
    check(fp != NULL, "could not open meta file");

    fwrite(&network->n_layers, sizeof(u32), 1, fp);
    fwrite(&network->n_neurons, sizeof(u32), 1, fp);
    fwrite(&network->n_synapses, sizeof(u32), 1, fp);

    // NOTE: write layer meta
    NetworkLayerLink* link = NULL;
    Layer* layer = NULL;
    String* layer_name = NULL;
    u32 layer_neuron_start_i = 0;

    for (link = network->layers.first; link != NULL; link = link->next) {
        layer = link->layer;
        layer_name = layer->name;
        layer_neuron_start_i = layer->neurons - network->neurons;

        fwrite(&layer_name->length, sizeof(u32), 1, fp);
        fwrite(string_get_c_str(layer_name), sizeof(char), layer_name->length, fp);
        fwrite(&layer_neuron_start_i, sizeof(u32), 1, fp);
        fwrite(&layer->n_neurons, sizeof(u32), 1, fp);
    }

    // NOTE: write synapses meta
    Synapse* synapse = NULL;
    u32 in_neuron_i = 0;
    u32 out_neuron_i = 0;
    for (u32 i = 0; i < network->n_synapses; ++i) {
        synapse = network->synapses + i;
        in_neuron_i = synapse->in_neuron - network->neurons;
        out_neuron_i = synapse->out_neuron - network->neurons;

        fwrite(&in_neuron_i, sizeof(u32), 1, fp);
        fwrite(&out_neuron_i, sizeof(u32), 1, fp);
    }

    fflush(fp);
    fclose(fp);

    // NOTE: Create the callback
    Callback* callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    callback->type = CALLBACK_META_DUMPER;
    callback->dumper_meta.meta_file = meta_file;
    callback->dumper_meta.sample_duration = 0;
    callback->dumper_meta.sample_time = 0;

    return callback;

    error:
    return NULL;
}


internal void
callback_meta_dumper_begin_sample(Callback* callback, DataSample* sample, Memory* memory) {
    DumperMeta* meta = &callback->dumper_meta;
    meta->sample_name = sample->name;
    meta->sample_duration = sample->duration;
    meta->sample_time = 0;
}


internal void
callback_meta_dumper_update(Callback* callback, u32 time, Memory* memory) {
    DumperMeta* meta =  &callback->dumper_meta;

    check(meta->sample_time < meta->sample_duration,
          "meta->sample_time (%u) >= meta->sample_duration (%u)",
          meta->sample_time, meta->sample_duration);
    ++(meta->sample_time);

    error:
    return;
}


internal void
callback_meta_dumper_end_sample(Callback* callback, Memory* memory) {
    DumperMeta* meta =  &callback->dumper_meta;

    // NOTE: append sample information
    FILE* fp = fopen(string_get_c_str(meta->meta_file), "ab");
    check(fp != NULL,
          "Could not open file %s",
          string_get_c_str(meta->meta_file));

    fwrite(&(meta->sample_name->length), sizeof(u32), 1, fp);
    fwrite(meta->sample_name->data, sizeof(char), meta->sample_name->length, fp);
    fwrite(&(meta->sample_duration), sizeof(u32), 1, fp);

    fflush(fp);
    fclose(fp);

    error:
    return;
}