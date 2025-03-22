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
    f32 dw = 0.0f;
    SynapseCls* cls = synapse->cls;
    LearningInfo* learning_info = &(cls->learning_info);
    if (learning_info->enable == FALSE) return;

    u32 in_neuron_spike_time = synapse->in_neuron->last_spike_time;
    u32 synapse_spike_time = synapse->last_spike_time;
    u32 out_neuron_spike_time = synapse->out_neuron->last_spike_time;

    // NOTE: no spike, no update
    if (synapse_spike_time == INVALID_SPIKE_TIME) return;
    if (out_neuron_spike_time == INVALID_SPIKE_TIME) return;

    // NOTE: synapse contribution is before the out neuron spiked -> Potentiation
    if (synapse_spike_time < out_neuron_spike_time)
    {
        u32 dt = out_neuron_spike_time - synapse_spike_time;

        if (learning_info->type == SYNAPSE_LEARNING_NO_LEARNING) {
            dw = 0;
        } else if (learning_info->type == SYNAPSE_LEARNING_EXPONENTIAL) {
            SynapseLearningExponential* rule = &(learning_info->stdp_exponential);
            dw = rule->A * math_exp_f32(-(f32)dt / rule->tau);
        } else if (learning_info->type == SYNAPSE_LEARNING_STEP) {
            SynapseLearningStep* rule = &(learning_info->stdp_step);
            if (dt <= rule->max_time_p) dw = rule->amp_p;
        } else {
            log_error("Unkown Synapse Learning Rule %s (%u)",
            synapse_learning_rule_get_c_str(learning_info->type),
            learning_info->type);
        }
    }

    // NOTE: synapse contribution is after the out neuron spiked -> Depression
    if (synapse_spike_time > out_neuron_spike_time)
    {
        u32 dt = synapse_spike_time - out_neuron_spike_time;

        if (learning_info->type == SYNAPSE_LEARNING_NO_LEARNING) {
            dw = 0.0f;
        } else if (learning_info->type == SYNAPSE_LEARNING_EXPONENTIAL) {
            SynapseLearningExponential* rule = &(learning_info->stdp_exponential);
            dw = rule->B * math_exp_f32(-(f32)dt / rule->tau);
        } else if (learning_info->type == SYNAPSE_LEARNING_STEP) {
            SynapseLearningStep* rule = &(learning_info->stdp_step);
            if (dt <= rule->max_time_d) dw = rule->amp_d;
        } else {
            log_error("Unkown Synapse Learning Rule %s (%u)",
            synapse_learning_rule_get_c_str(learning_info->type),
            learning_info->type);
        }
    }

    // NOTE: if synapse spiked at the same time as the output neuron, no conclusion

    synapse->weight = math_clip_f32(synapse->weight + dw, learning_info->min_w, learning_info->max_w);
}

internal void
callback_stdp_v1_end_sample(Callback* callback, Memory* memory)
{

}