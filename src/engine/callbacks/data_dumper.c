internal Callback*
callback_network_data_dumper_create(Memory* memory, String* output_folder, Network* network) {
    check(memory != NULL, "memory is NULL");
    check(output_folder != NULL, "output_folder is NULL");
    check(network != NULL, "network is NULL");

    b32 result = os_folder_create_str(output_folder);
    check(result == TRUE, "couldn't create folder %s",
          string_get_c_str(output_folder));

    Callback* callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    DumperNeuronData* neuron_data = (DumperNeuronData*)memory_push(
        memory,
        sizeof(*neuron_data) * network->n_neurons);
    check_memory(neuron_data);

    DumperSynapseData* synapse_data = (DumperSynapseData*)memory_push(
        memory,
        sizeof(*synapse_data) * network->n_synapses);
    check_memory(synapse_data);

    callback->type = CALLBACK_NETWORK_DATA_DUMPER;
    callback->dumper_data.network = network;
    callback->dumper_data.output_folder = output_folder;
    callback->dumper_data.sample_fp = NULL;
    callback->dumper_data.neuron_data = neuron_data;
    callback->dumper_data.synapse_data = synapse_data;

    return callback;

    error:
    return NULL;
}


internal void
callback_network_data_dumper_begin_sample(Callback* callback, DataSample* sample, Memory* memory) {
    DumperData* data = &callback->dumper_data;

    char file_name[100];
    sprintf(file_name, "data_%s.bin", string_get_c_str(sample->name));
    String* file_name_s = string_create(memory, file_name);
    check_memory(file_name_s);

    String* file_path = string_path_join(memory,
                                         data->output_folder,
                                         file_name_s);
    check(file_path != NULL, "Couldn't build path for sample file %s", file_name);

    data->sample_fp = fopen(string_get_c_str(file_path), "wb");
    check(data->sample_fp != NULL,
          "Could not open sample file %s",
          string_get_c_str(file_path));

    error:
    return;
}


internal void
callback_network_data_dumper_update(Callback* callback, u32 time, Memory* memory) {
    DumperData* data = &callback->dumper_data;

    // NOTE: dump the neurons
    u32 neuron_i = 0;
    Neuron* neuron = NULL;
    Network* network = data->network;
    DumperNeuronData* neuron_data = NULL;
    Neuron* neurons = network->neurons;
    DumperNeuronData* neurons_data = data->neuron_data;

    for (neuron_i = 0; neuron_i < network->n_neurons; ++neuron_i) {
        neuron = neurons + neuron_i;
        neuron_data = neurons_data + neuron_i;

        neuron_data->voltage = neuron->voltage;
        neuron_data->spike = neuron->spike;
        neuron_data->psc = neuron->epsc + neuron->ipsc;
        neuron_data->epsc = neuron->epsc;
        neuron_data->ipsc = neuron->ipsc;
    }
    fwrite(neurons_data,
            sizeof(DumperNeuronData),
            network->n_neurons,
            data->sample_fp);

    // NOTE: dump the synapses
    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    DumperSynapseData* synapse_data = NULL;
    Synapse* synapses = network->synapses;
    DumperSynapseData* synapses_data = data->synapse_data;

    for (synapse_i = 0; synapse_i < network->n_synapses; ++synapse_i) {
        synapse = synapses + synapse_i;
        synapse_data = synapses_data + synapse_i;

        synapse_data->weight = synapse->weight;
        synapse_data->conductance = synapse->conductance;
    }
    fwrite(synapses_data,
           sizeof(DumperSynapseData),
           network->n_synapses,
           data->sample_fp);
}


internal void
callback_network_data_dumper_end_sample(Callback* callback, Memory* memory) {
    DumperData* data = &callback->dumper_data;

    fflush(data->sample_fp);
    fclose(data->sample_fp);
}
