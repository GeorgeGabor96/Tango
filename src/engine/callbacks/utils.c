internal b32
callback_utils_get_reward_first_spike(Network* network, DataSample* sample)
{
    b32 reward = FALSE;
    u32 neuron_winner_i = -1;
    u32 neuron_winner_first_spike_time = -1;
    b8 found = FALSE;

    check(network->n_out_layers == 1, "Only support one output layer for now");
    Layer* out_layer = network->out_layers.first->layer;


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
    if (found == TRUE)
    {
        if (neuron_winner_i == sample->winner_neuron_i)
        {
            reward = TRUE;
        }
    }

    error:
    return reward;
}

internal u32
callback_utils_get_winner_neuron(Network* net, DataSample* sample)
{
    u32 neuron_winner_i = -1;
    u32 neuron_winner_first_spike_time = -1;
    b8 found = FALSE;

    check(net->n_out_layers == 1, "Only support one output layer for now");
    Layer* out_layer = net->out_layers.first->layer;

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

    error:
    return neuron_winner_i;
}