

typedef struct DumperNeuronData {
    f32 voltage;
    b32 spike;
    f32 psc;
    f32 epsc;
    f32 ipsc;
} DumperNeuronData;

typedef struct DumperSynapseData {
    f32 weight;
    f32 conductance;
} DumperSynapseData;

typedef struct DumperData {
    String* file_name;
    FILE* fp;

    DumperNeuronData* neuron_data;
    DumperSynapseData* synapse_data;
} DumperData;


typedef struct Dumper {
    String* output_folder;
    Network* network;

    u32 sample_step;
    u32 sample_count;
    u32 sample_duration;

    DumperMeta* meta;
    DumperData* data;
} Dumper;


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


internal Callback*
callback_dumper_create(Memory* memory, String* output_folder, Network* network) {
    check(memory != NULL, "memory is NULL");
    check(output_folder != NULL, "output_folder is NULL");
    check(network != NULL, "network is NULL");

    Callback* callback = NULL;

    callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    DumperMeta* meta = _callback_dumper_build_meta(network, memory, output_folder);
    check_memory(meta);

    DumperData* data = _callback_dumper_build_data(meta, memory);
    check_memory(data);

    callback->type = CALLBACK_NETWORK_DUMPER;
    callback->dumper.output_folder = output_folder;
    callback->dumper.sample_step = 0;
    callback->dumper.sample_count = 0;
    callback->dumper.sample_duration = 0;
    callback->dumper.meta = meta;
    callback->dumper.data = data;
    callback->dumper.network = network;

    b32 result = os_folder_create_str(output_folder);
    check(result == TRUE, "couldn't create folder %s",
          string_get_c_str(output_folder));

    _callback_dumper_save_meta(meta);

    return callback;

    error:
    return NULL;
}


internal void
callback_dumper_begin_sample(Callback* callback, u32 sample_duration, Memory* memory) {
    Dumper* dumper = NULL;
    check(callback != NULL, "callback is NULL");
    check(callback->type == CALLBACK_NETWORK_DUMPER,
          "callback->type should be %u (%s) not %u (%s)",
          CALLBACK_NETWORK_DUMPER,
          callback_type_get_c_str(CALLBACK_NETWORK_DUMPER),
          callback->type,
          callback_type_get_c_str(callback->type));
    check(sample_duration != 0, "sample_duration is 0");
    check(memory != NULL, "memory is NULL");

    dumper = &callback->dumper;
    dumper->sample_duration = sample_duration;
    dumper->sample_step = 0;

    char file_name[100];
    sprintf(file_name, "sample_%d.bin", dumper->sample_count);


    dumper->data->file_name = string_create(memory, file_name);
    check_memory(file_name);

    String* file_path = string_path_join(memory,
                                         dumper->output_folder,
                                         dumper->data->file_name);
    check(file_path != NULL, "Couldn't build path for sample file %s", file_name);

    dumper->data->fp = fopen(string_get_c_str(file_path), "wb");
    check(dumper->data->fp != NULL, "Could not open sample file %s", string_get_c_str(file_path));

    error:
    return;
}


internal void
callback_dumper_update(Callback* callback, Memory* memory) {
    check(callback != NULL, "dumper is NULL");
    check(callback->type == CALLBACK_NETWORK_DUMPER,
          "callback->type should be %u (%s) not %u (%s)",
          CALLBACK_NETWORK_DUMPER,
          callback_type_get_c_str(CALLBACK_NETWORK_DUMPER),
          callback->type,
          callback_type_get_c_str(callback->type));
    check(memory != NULL, "memory is NULL");

    Dumper* dumper = &callback->dumper;
    check(dumper->sample_step < dumper->sample_duration,
          "More sample steps (%u) than expected (%u)",
          dumper->sample_step, dumper->sample_duration);
    ++(dumper->sample_step);

    // NOTE: dump the neurons
    u32 neuron_i = 0;
    Neuron* neuron = NULL;
    Network* network = dumper->network;
    DumperNeuronData* neuron_data = NULL;
    Neuron* neurons = network->neurons;
    DumperNeuronData* neurons_data = dumper->data->neuron_data;

    for (neuron_i = 0; neuron_i < network->n_neurons; ++neuron_i) {
        neuron = neurons + neuron_i;
        neuron_data = neurons_data + neuron_i;

        neuron_data->voltage = neuron->voltage;
        neuron_data->spike = neuron->spike;
        neuron_data->psc = neuron->epsc + neuron->ipsc;
        neuron_data->epsc = neuron->epsc;
        neuron_data->ipsc = neuron->ipsc;
    }
    fwrite(neurons_data, sizeof(DumperNeuronData), dumper->meta->n_neurons, dumper->data->fp);

    // NOTE: dump the synapses
    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    DumperSynapseData* synapse_data = NULL;
    Synapse* synapses = network->synapses;
    DumperSynapseData* synapses_data = dumper->data->synapse_data;

    for (synapse_i = 0; synapse_i < network->n_synapses; ++synapse_i) {
        synapse = synapses + synapse_i;
        synapse_data = synapses_data + synapse_i;

        synapse_data->weight = synapse->weight;
        synapse_data->conductance = synapse->conductance;
    }
    fwrite(synapses_data, sizeof(DumperSynapseData), dumper->meta->n_synapses, dumper->data->fp);

    error:
    return;
}


internal void
callback_dumper_end_sample(Callback* callback, Memory* memory) {
    check(callback != NULL, "dumper is NULL");
    check(callback->type == CALLBACK_NETWORK_DUMPER,
          "callback->type should be %u (%s) not %u (%s)",
          CALLBACK_NETWORK_DUMPER,
          callback_type_get_c_str(CALLBACK_NETWORK_DUMPER),
          callback->type,
          callback_type_get_c_str(callback->type));
    check(memory != NULL, "memory is NULL");

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
