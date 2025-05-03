internal Callback*
callback_accuracy_create(Memory* memory, String* output_folder, Network* network)
{
    check(memory != NULL, "memory is NULL");
    check(output_folder != NULL, "output_folder is NULL");
    check(network != NULL, "network is NULL");

    b32 result = os_folder_create_str(output_folder);
    check(result == TRUE, "couldn't create folder %s", string_get_c_str(output_folder));

    char file_name[100] = { 0 };
    sprintf(file_name, "accuracy.txt");
    String* file_name_s = string_create(memory, file_name);
    check_memory(file_name_s);

    String* file_path = string_path_join(memory, output_folder, file_name_s);
    check(file_path != NULL, "Couldn't build path for sample file %s", file_name);

    FILE* fp = fopen(string_get_c_str(file_path), "w");
    check(fp != NULL, "Could not open sample file %s", string_get_c_str(file_path));

    Callback* callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    callback->type = CALLBACK_ACCURACY;
    callback->network = network;
    callback->accuracy.hits = 0;
    callback->accuracy.total = 0;
    callback->accuracy.fp = fp;

    return callback;

    error:
    return NULL;
}


internal CALLBACK_BEGIN_SAMPLE(callback_accuracy_begin_sample)
{

}


internal CALLBACK_UPDATE(callback_accuracy_update)
{

}


internal CALLBACK_END_SAMPLE(callback_accuracy_end_sample)
{
    // find the winner neuron of the network
    Network* network = callback->network;

    check(network->n_out_layers == 1, "Only support one output layer for now");
    Layer* out_layer = network->out_layers.first->layer;

    u32 neuron_winner_i = 0;
    u32 neuron_winner_first_spike_time = 0;
    b8 found = FALSE;

    for (int neuron_i = 0; neuron_i < out_layer->n_neurons; ++neuron_i)
    {
        Neuron* neuron = out_layer->neurons + neuron_i;

        if (neuron->n_spikes > 0)
        {
            u32 neuron_first_spike = neuron->spike_times[0];
            if (neuron_first_spike < neuron_winner_first_spike_time)
            {
                neuron_winner_first_spike_time = neuron_first_spike;
                neuron_winner_i = neuron_i;
            }
            found = TRUE;
        }
    }

    // compare it to what was expected and update the metrics
    Accuracy* accuracyData = &callback->accuracy;
    if (found == TRUE)
    {
        if (neuron_winner_i == sample->winner_neuron_i)
        {
            accuracyData->hits++;
        }
    }
    accuracyData->total++;

    error:
    return;
}


internal CALLBACK_BEGIN_EPOCH(callback_accuracy_begin_epoch)
{
    callback->accuracy.hits = 0;
    callback->accuracy.total = 0;
}


internal CALLBACK_END_EPOCH(callback_accuracy_end_epoch)
{
    Accuracy* accuracyData = &callback->accuracy;
    f32 accuracy = accuracyData->hits / (accuracyData->total + 0.00001);
    log_info("\n--------------------------\nEPOCH %u Accurary: %.2f\n--------------------------", epoch_i, accuracy);

    fprintf(accuracyData->fp, "%.02f\n", accuracy);
}


internal CALLBACK_BEGIN_EXPERIMENT(callback_accuracy_begin_experiment)
{

}


internal CALLBACK_END_EXPERIMENT(callback_accuracy_end_experiment)
{
    Accuracy* accuracy = &callback->accuracy;
    fflush(accuracy->fp);
    fclose(accuracy->fp);
}