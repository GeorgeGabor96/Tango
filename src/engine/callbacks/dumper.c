internal DumperMeta*
_callback_dumper_build_meta(Network* network, Memory* memory, String* out_path) {
    DumperMeta* meta = (DumperMeta*)memory_push(memory, sizeof(*meta));
    check_memory(meta);

    // NOTE: find the number of entities
    u32 n_layers = network->n_layers;
    u32 n_neurons = 0;
    u32 n_synapses = 0;
    u32 layer_i = 0;
    u32 neuron_i = 0;
    u32 in_synapses_i = 0;
    u32 synapse_i = 0;
    Layer* layer = NULL;
    Neuron* neuron = NULL;
    InSynapseArray* in_synapses = NULL;

    for (layer_i = 0; layer_i < n_layers; ++layer_i) {
        layer = network->layers[layer_i];
        n_neurons += layer->n_neurons;

        for (neuron_i = 0; neuron_i < layer->n_neurons; ++neuron_i) {
            neuron = &layer->neurons[neuron_i];

            for (in_synapses_i = 0; in_synapses_i < neuron->n_in_synapse_arrays; ++in_synapses_i) {
                in_synapses = neuron->in_synapse_arrays[in_synapses_i];
                n_synapses += in_synapses->length;
            }
        }
    }

    // NOTE: build layer meta
    DumperLayerMeta* layers_meta = (DumperLayerMeta*)memory_push(memory,
                                                                 sizeof(*layers_meta) * n_layers);
    check_memory(layers_meta);

    u32 neuron_idx = 0;
    DumperLayerMeta* layer_meta = NULL;
    for (layer_i = 0; layer_i < n_layers; ++layer_i) {
        layer = network->layers[layer_i];
        layer_meta = &layers_meta[layer_i];

        layer_meta->name = layer->name;
        layer_meta->neuron_start_idx = neuron_idx;
        layer_meta->n_neurons = layer->n_neurons;
        neuron_idx += layer->n_neurons;
    }

    // NOTE: build synapse meta
    DumperSynapseMeta* synapses_meta = (DumperSynapseMeta*)memory_push(memory,
                                                           sizeof(*synapses_meta) * n_synapses);
    check_memory(synapses_meta);

    u32 synapse_idx = 0;
    Layer* out_layer = NULL;
    Layer* in_layer = NULL;
    Neuron* out_neuron = NULL;
    Synapse* synapse = NULL;
    DumperLayerMeta* in_layer_meta = NULL;
    DumperLayerMeta* out_layer_meta = NULL;
    u32 in_layer_idx = 0;
    u32 in_neuron_idx = 0;
    u32 out_neuron_idx = 0;

    for (layer_i = 0; layer_i < n_layers; ++layer_i) {
        out_layer = network->layers[layer_i];
        out_layer_meta = &layers_meta[layer_i];

        for (neuron_i = 0; neuron_i < out_layer->n_neurons; ++neuron_i) {
            out_neuron = &out_layer->neurons[neuron_i];
            // The output neuron is the one that has the synapse as an input
            out_neuron_idx = out_layer_meta->neuron_start_idx +
                             layer_get_neuron_idx(out_layer, out_neuron);

            for (in_synapses_i = 0;
                 in_synapses_i < out_neuron->n_in_synapse_arrays;
                 ++in_synapses_i) {

                in_synapses = out_neuron->in_synapse_arrays[in_synapses_i];
                // The index of the in_synapses and the idx of input layer of the current layer should match
                in_layer = out_layer->in_layers[in_synapses_i];
                in_layer_idx = network_get_layer_idx(network, in_layer);
                in_layer_meta = &layers_meta[in_layer_idx];

                for (synapse_i = 0; synapse_i < in_synapses->length; ++synapse_i) {
                    synapse = in_synapse_array_get(in_synapses, synapse_i);
                    in_neuron_idx = in_layer_meta->neuron_start_idx +
                                    layer_get_neuron_idx(in_layer, synapse->in_neuron);

                    synapses_meta[synapse_idx].in_neuron_idx = in_neuron_idx;
                    synapses_meta[synapse_idx].out_neuron_idx = out_neuron_idx;
                    ++synapse_idx;
                }
            }
        }
    }

    // Build the file path
    String* file_path = string_path_join_c_str(memory, out_path, "meta.bin");
    check_memory(file_path);

    meta->file_path = file_path;
    meta->n_layers = n_layers;
    meta->n_neurons = n_neurons;
    meta->n_synapses = n_synapses;
    meta->layer_meta = layers_meta;
    meta->synapse_meta = synapses_meta;
    return meta;

    error:
    return NULL;
}


internal DumperData*
_callback_dumper_build_data(DumperMeta* meta, Memory* memory) {
    DumperData* data = (DumperData*)memory_push(memory, sizeof(*data));
    check_memory(data);

    DumperNeuronData* neuron_data = (DumperNeuronData*)memory_push(memory,
                                    sizeof(*neuron_data) * meta->n_neurons);
    check_memory(neuron_data);

    DumperSynapseData* synapse_data = (DumperSynapseData*)memory_push(memory,
                                      sizeof(*synapse_data) * meta->n_synapses);
    check_memory(synapse_data);

    data->file_name = NULL;
    data->neuron_data = neuron_data;
    data->synapse_data = synapse_data;
    return data;

    error:
    return NULL;
}


internal void
_callback_dumper_save_meta(DumperMeta* meta) {
    u32 i = 0;
    String* name = NULL;
    DumperSynapseMeta* s_meta = NULL;
    DumperLayerMeta* l_meta = NULL;
    FILE* fp = fopen(string_get_c_str(meta->file_path), "wb");
    check(fp != NULL, "could not open meta file");

    fwrite(&meta->n_layers, sizeof(u32), 1, fp);
    fwrite(&meta->n_neurons, sizeof(u32), 1, fp);
    fwrite(&meta->n_synapses, sizeof(u32), 1, fp);

    for (i = 0; i < meta->n_layers; ++i) {
        l_meta = &meta->layer_meta[i];
        name = l_meta->name;

        fwrite(&name->length, sizeof(u32), 1, fp);
        fwrite(string_get_c_str(name), sizeof(char), name->length, fp);
        fwrite(&l_meta->neuron_start_idx, sizeof(u32), 1, fp);
        fwrite(&l_meta->n_neurons, sizeof(u32), 1, fp);
    }

    for (i = 0; i < meta->n_synapses; ++i) {
        s_meta = &meta->synapse_meta[i];

        fwrite(&s_meta->in_neuron_idx, sizeof(u32), 1, fp);
        fwrite(&s_meta->out_neuron_idx, sizeof(u32), 1, fp);
    }

    fflush(fp);
    fclose(fp);

    error:
    return;
}


internal Callback*
callback_dumper_create(State* state, const char* output_folder, Network* network) {
    check(state != NULL, "state is NULL");
    check(output_folder != NULL, "output_folder is NULL");
    check(network != NULL, "network is NULL");

    Callback* callback = NULL;
    String* output_folder_s = NULL;

    output_folder_s = string_create(state->permanent_storage, output_folder);
    check_memory(output_folder_s);

    callback = (Callback*)memory_push(state->permanent_storage, sizeof(*callback));
    check_memory(callback);

    DumperMeta* meta = _callback_dumper_build_meta(network,
                                state->permanent_storage,
                                output_folder_s);
    check_memory(meta);

    DumperData* data = _callback_dumper_build_data(meta, state->permanent_storage);
    check_memory(data);

    callback->type = CALLBACK_NETWORK_DUMPER;
    callback->dumper.output_folder = output_folder_s;
    callback->dumper.sample_step = 0;
    callback->dumper.sample_count = 0;
    callback->dumper.sample_duration = 0;
    callback->dumper.meta = meta;
    callback->dumper.data = data;

    bool result = os_folder_create_str(output_folder_s);
    check(result == TRUE, "couldn't create folder %s",
          string_get_c_str(output_folder_s));

    _callback_dumper_save_meta(meta);

    return callback;

    error:
    return NULL;
}


internal void
callback_dumper_begin_sample(State* state,
                             Callback* callback,
                             Network* network,
                             u32 sample_duration) {
    Dumper* dumper = NULL;
    check(state != NULL, "state is NULL");
    check(callback != NULL, "callback is NULL");
    check(callback->type == CALLBACK_NETWORK_DUMPER,
          "callback->type should be %u (%s) not %u (%s)",
          CALLBACK_NETWORK_DUMPER,
          callback_type_get_c_str(CALLBACK_NETWORK_DUMPER),
          callback->type,
          callback_type_get_c_str(callback->type));
    check(network != NULL, "network is NULL");
    check(sample_duration != 0, "sample_duration is 0");

    dumper = &callback->dumper;
    dumper->sample_duration = sample_duration;
    dumper->sample_step = 0;

    char file_name[100];
    sprintf(file_name, "sample_%d.bin", dumper->sample_count);


    dumper->data->file_name = string_create(state->transient_storage, file_name);
    check_memory(file_name);

    String* file_path = string_path_join(state->transient_storage,
                                         dumper->output_folder,
                                         dumper->data->file_name);
    check(file_path != NULL, "Couldn't build path for sample file %s", file_name);

    dumper->data->fp = fopen(string_get_c_str(file_path), "wb");
    check(dumper->data->fp != NULL, "Could not open sample file %s", string_get_c_str(file_path));

    error:
    return;
}


internal void
callback_dumper_update(State* state, Callback* callback, Network* network) {
    check(state != NULL, "state is NULL");
    check(callback != NULL, "dumper is NULL");
    check(callback->type == CALLBACK_NETWORK_DUMPER,
          "callback->type should be %u (%s) not %u (%s)",
          CALLBACK_NETWORK_DUMPER,
          callback_type_get_c_str(CALLBACK_NETWORK_DUMPER),
          callback->type,
          callback_type_get_c_str(callback->type));
    check(network != NULL, "network is NULL");

    Dumper* dumper = &callback->dumper;
    check(dumper->sample_step < dumper->sample_duration,
          "More sample steps (%u) than expected (%u)",
          dumper->sample_step, dumper->sample_duration);

    u32 neuron_idx = 0;
    u32 synapse_idx = 0;
    u32 layer_i = 0;
    u32 neuron_i = 0;
    u32 in_synapses_i = 0;
    u32 synapse_i = 0;
    Layer* layer = NULL;
    Neuron* neuron = NULL;
    InSynapseArray* in_synapses = NULL;
    Synapse* synapse = NULL;
    DumperNeuronData* neuron_data = NULL;
    DumperSynapseData* synapse_data = NULL;

    for (layer_i = 0; layer_i < network->n_layers; ++layer_i) {
        layer = network->layers[layer_i];

        for (neuron_i = 0; neuron_i < layer->n_neurons; ++neuron_i) {
            neuron = &layer->neurons[neuron_i];

            neuron_data = &dumper->data->neuron_data[neuron_idx];
            neuron_data->voltage = neuron->voltage;
            neuron_data->spike = neuron->spike;
            neuron_data->psc = neuron->epsc + neuron->ipsc;
            neuron_data->epsc = neuron->epsc;
            neuron_data->ipsc = neuron->ipsc;

            ++neuron_idx;

            for (in_synapses_i = 0; in_synapses_i < neuron->n_in_synapse_arrays; ++in_synapses_i) {
                in_synapses = neuron->in_synapse_arrays[in_synapses_i];

                for (synapse_i = 0; synapse_i < in_synapses->length; ++synapse_i) {
                    synapse = in_synapse_array_get(in_synapses, synapse_i);

                    synapse_data = &dumper->data->synapse_data[synapse_idx];
                    synapse_data->weight = synapse->weight;
                    synapse_data->conductance = synapse->conductance;

                    ++synapse_idx;
                }
            }
        }
    }

    fwrite(dumper->data->neuron_data, sizeof(DumperNeuronData), dumper->meta->n_neurons,
           dumper->data->fp);
    fwrite(dumper->data->synapse_data, sizeof(DumperSynapseData), dumper->meta->n_synapses,
           dumper->data->fp);

    ++(dumper->sample_step);

    error:
    return;
}


internal void
callback_dumper_end_sample(State* state, Callback* callback, Network* network) {
    check(state != NULL, "state is NULL");
    check(callback != NULL, "dumper is NULL");
    check(callback->type == CALLBACK_NETWORK_DUMPER,
          "callback->type should be %u (%s) not %u (%s)",
          CALLBACK_NETWORK_DUMPER,
          callback_type_get_c_str(CALLBACK_NETWORK_DUMPER),
          callback->type,
          callback_type_get_c_str(callback->type));
    check(network != NULL, "network is NULL");

    Dumper* dumper = &callback->dumper;

    // NOTE: 1. Need to close the data file
    fflush(dumper->data->fp);
    fclose(dumper->data->fp);
    log_info("Finished dumping in %s", string_get_c_str(dumper->data->file_name));

    // NOTE: 2. Need to write the sample duration in the meta file for this sample file
    // Need to open the meta file and close it after
    FILE* fp = fopen(string_get_c_str(dumper->meta->file_path), "ab"); // need to join
    check(fp != NULL, "Could not open file %s", string_get_c_str(dumper->meta->file_path));

    String* file_name = dumper->data->file_name;

    fwrite(&(file_name->length), sizeof(u32), 1, fp);
    fwrite(file_name->data, sizeof(char), file_name->length, fp);
    fwrite(&(dumper->sample_duration), sizeof(u32), 1, fp);

    fflush(fp);
    fclose(fp);

    ++(dumper->sample_count);

    error:
    return;
}
