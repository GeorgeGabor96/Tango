internal Callback* callback_learning_custom_create(Memory* memory, Network* network)
{
    check(memory != NULL, "memory is NULL");
    check(network != NULL, "network is NULL");

    Callback* callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    callback->type = CALLBACK_LEARNING_CUSTOM;
    callback->network = network;
    callback->learning_custom.data = NULL;

    return callback;

    error:
    return NULL;
}

internal CALLBACK_BEGIN_SAMPLE(callback_learning_custom_begin_sample)
{

}

internal CALLBACK_UPDATE(callback_learning_custom_update)
{

}

internal CALLBACK_END_SAMPLE(callback_learning_custom_end_sample)
{
    // NOTE: Case 1 -> network correct answer -> No changes
    if (reward == TRUE)
    {
        return;
    }

    // NOTE: Case 2 -> network has wrong answer
    // 1. find winner and drepresse its synapses
    // 2. find neuron that should have won and potentiate its synapses

    // TODO: the amount of change can be dependend on the time between winner - should be winner
    // the bigger the difference (winner spiked much faster than should_winner) the more changed required
    u32 winner_i = callback_utils_get_winner_neuron(callback->network, sample);
    if (winner_i != -1)
    {
        Neuron* neuron = callback->network->out_layers.first->layer->neurons + winner_i;

        u32 synapse_i = 0;
        Synapse* synapse = NULL;
        SynapseRefArray* it = NULL;
        for (it = neuron->in_synapse_arrays; it != NULL; it = it->next)
        {
            for (synapse_i = 0; synapse_i < it->length; ++synapse_i)
            {
                synapse = it->synapses[synapse_i];
                synapse->weight -= 0.05;
            }
        }
    }

    u32 should_be_winner_i = sample->winner_neuron_i;
    if (should_be_winner_i != -1)
    {
        Neuron* neuron = callback->network->out_layers.first->layer->neurons + winner_i;

        u32 synapse_i = 0;
        Synapse* synapse = NULL;
        SynapseRefArray* it = NULL;
        for (it = neuron->in_synapse_arrays; it != NULL; it = it->next)
        {
            for (synapse_i = 0; synapse_i < it->length; ++synapse_i)
            {
                synapse = it->synapses[synapse_i];
                synapse->weight += 0.05;
            }
        }
    }
}

internal CALLBACK_BEGIN_EPOCH(callback_learning_custom_begin_epoch)
{

}

internal CALLBACK_END_EPOCH(callback_learning_custom_end_epoch)
{

}

internal CALLBACK_BEGIN_EXPERIMENT(callback_learning_custom_begin_experiment)
{

}

internal CALLBACK_END_EXPERIMENT(callback_learning_custom_end_experiment)
{

}