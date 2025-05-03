internal Callback*
callback_accuracy_create(Memory* memory, Network* network)
{
    Callback* callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    callback->type = CALLBACK_ACCURACY;
    callback->network = network;
    callback->accuracy.hits = 0;
    callback->accuracy.total = 0;

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
}


internal CALLBACK_BEGIN_EXPERIMENT(callback_accuracy_begin_experiment)
{

}


internal CALLBACK_END_EXPERIMENT(callback_accuracy_end_experiment)
{

}