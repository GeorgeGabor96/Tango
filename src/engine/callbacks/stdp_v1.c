internal Callback*
callback_stdp_v1_create(Memory* memory, Network* network)
{
    check(memory != NULL, "memory is NULL");
    check(network != NULL, "network is NULL");

    Callback* callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    callback->type = CALLBACK_STDP_V1;
    callback->stdp_v1.network = network;

    return callback;

    error:
    return NULL;
}


internal void
callback_stdp_v1_begin_sample(Callback* callback, DataSample* sample, Memory* memory)
{

}

static void _callback_stdp_v1_layer_update(Layer* layer);
static void _callback_stdp_v1_neuron_update(Neuron* neuron);
static void _callback_stdp_v1_synapse_update(Synapse* synapse);

internal void
callback_stdp_v1_update(Callback* callback, u32 time, Memory* memory)
{
    STDPv1* data = &callback->stdp_v1;

    Network* net = data->network;
    NetworkLayerLink* it = NULL;

    for (it = net->layers.first; it != NULL; it = it->next)
    {
        _callback_stdp_v1_layer_update(it->layer);
    }
}

static void
_callback_stdp_v1_layer_update(Layer* layer)
{
    for (u32 i = 0; i < layer->n_neurons; ++i)
    {
        Neuron* neuron = &(layer->neurons[i]);
        _callback_stdp_v1_neuron_update(neuron);
    }
}

static void
_callback_stdp_v1_neuron_update(Neuron* neuron)
{
    SynapseRefArray* it = NULL;
    u32 i = 0;
    for (it = neuron->in_synapse_arrays; it != NULL; it = it->next)
    {
        for (i = 0; i < it->length; ++i)
        {
            Synapse* synapse = it->synapses[i];
            _callback_stdp_v1_synapse_update(synapse);
        }
    }
}

static void
_callback_stdp_v1_synapse_update(Synapse* synapse)
{
    u32 in_neuron_spike_time = synapse->in_neuron->last_spike_time;
    u32 syanpse_spike_time = synapse->last_spike_time;
    u32 out_neuron_spike_time = synapse->out_neuron->last_spike_time;

    // TODO: actual stdp logic
}

internal void
callback_stdp_v1_end_sample(Callback* callback, Memory* memory)
{

}