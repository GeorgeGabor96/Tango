internal Callback*
callback_stdp_v1_create(Memory* memory, Network* network, u8 cooldown_value)
{
    check(memory != NULL, "memory is NULL");
    check(network != NULL, "network is NULL");

    Callback* callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    b8* cooldown = (b8*)memory_push(memory, sizeof(b8) * network->n_synapses);
    check_memory(cooldown);
    for (u32 i = 0; i < network->n_synapses; ++i)
    {
        cooldown[i] = 0;
    }

    callback->type = CALLBACK_STDP_V1;
    callback->network = network;
    callback->stdp_v1.cooldown = cooldown;
    callback->stdp_v1.cooldown_value = cooldown_value;

    return callback;

    error:
    return NULL;
}


internal void
callback_stdp_v1_begin_sample(Callback* callback, DataSample* sample, Memory* memory)
{

}

static b8 _callback_stdp_v1_synapse_update(Synapse* synapse, Inputs* inputs, Network* network);

internal void
callback_stdp_v1_update(Callback* callback, Inputs* inputs, u32 time, Memory* memory)
{
    STDPv1* data = &callback->stdp_v1;
    Network* net = callback->network;

    for (u32 i = 0; i < net->n_synapses; ++i)
    {
        if (data->cooldown[i] > 0)
        {
            data->cooldown[i] -= 1;
            continue;
        }

        Synapse* synapse = net->synapses + i;
        b8 did_change = _callback_stdp_v1_synapse_update(synapse, inputs, callback->network);
        if (did_change)
        {
            data->cooldown[i] = data->cooldown_value;
        }
    }
}

b32 _get_reward(Network* network, Inputs* inputs);
f32 _r_stdp_potentiation_learning_rule(Synapse* synapse, LearningInfo* learning_info, b32 reward);
f32 _r_stdp_depression_learning_rule(Synapse* synapse, LearningInfo* learning_info, b32 reward);

static b8
_callback_stdp_v1_synapse_update(Synapse* synapse, Inputs* inputs, Network* network)
{
    f32 dw = 0.0f;
    SynapseCls* cls = synapse->cls;
    LearningInfo* learning_info = &(cls->learning_info);
    if (learning_info->enable == FALSE) return FALSE;

    u32 in_neuron_spike_time = synapse->in_neuron->last_spike_time;
    u32 synapse_spike_time = synapse->last_spike_time;
    u32 out_neuron_spike_time = synapse->out_neuron->last_spike_time;

    // NOTE: no spike, no update
    if (synapse_spike_time == INVALID_SPIKE_TIME) return FALSE;
    if (out_neuron_spike_time == INVALID_SPIKE_TIME) return FALSE;

    b32 reward = _get_reward(network, inputs);

    // NOTE: synapse contribution is before the out neuron spiked -> Potentiation
    if (synapse_spike_time < out_neuron_spike_time)
    {
        u32 dt = out_neuron_spike_time - synapse_spike_time;

        if (learning_info->type == SYNAPSE_LEARNING_NO_LEARNING)
        {
            dw = 0;
        }
        else if (learning_info->type == SYNAPSE_LEARNING_EXPONENTIAL)
        {
            SynapseLearningExponential* rule = &(learning_info->stdp_exponential);
            dw = rule->A * math_exp_f32(-(f32)dt / rule->tau);
        }
        else if (learning_info->type == SYNAPSE_LEARNING_STEP)
        {
            SynapseLearningStep* rule = &(learning_info->stdp_step);
            if (dt <= rule->max_time_p) dw = rule->amp_p;
        }
        else if (learning_info->type == SYNAPSE_LEARNING_RSTDP_EXPONENTIAL)
        {
            dw = _r_stdp_potentiation_learning_rule(synapse, learning_info, reward);
        }
        else
        {
            log_error("Unkown Synapse Learning Rule %s (%u)",
            synapse_learning_rule_get_c_str(learning_info->type),
            learning_info->type);
        }
    }

    // NOTE: synapse contribution is after the out neuron spiked -> Depression
    if (synapse_spike_time > out_neuron_spike_time)
    {
        u32 dt = synapse_spike_time - out_neuron_spike_time;

        if (learning_info->type == SYNAPSE_LEARNING_NO_LEARNING)
        {
            dw = 0.0f;
        }
        else if (learning_info->type == SYNAPSE_LEARNING_EXPONENTIAL)
        {
            SynapseLearningExponential* rule = &(learning_info->stdp_exponential);
            dw = rule->B * math_exp_f32(-(f32)dt / rule->tau);
        }
        else if (learning_info->type == SYNAPSE_LEARNING_STEP)
        {
            SynapseLearningStep* rule = &(learning_info->stdp_step);
            if (dt <= rule->max_time_d) dw = rule->amp_d;
        }
        else if (learning_info->type == SYNAPSE_LEARNING_RSTDP_EXPONENTIAL)
        {
            dw = _r_stdp_depression_learning_rule(synapse, learning_info, reward);
        }
        else
        {
            log_error("Unkown Synapse Learning Rule %s (%u)",
            synapse_learning_rule_get_c_str(learning_info->type),
            learning_info->type);
        }
    }

    // NOTE: if synapse spiked at the same time as the output neuron, no conclusion
    b8 w_changed = FALSE;
    if (dw != 0) {
        synapse->weight = math_clip_f32(synapse->weight + dw, learning_info->min_w, learning_info->max_w);
        w_changed = TRUE;
    }
    return w_changed;
}

internal void
callback_stdp_v1_end_sample(Callback* callback, DataSample* sample, Memory* memory)
{

}


b32 _get_reward(Network* network, Inputs* inputs)
{
    b32 reward = TRUE;
    check(network->n_out_layers == 1, "Only one output layers for now");

    u32 network_winner_neuron_i = 0;
    b32 network_winner_is_valid = FALSE;

    Layer* output_layer = network->out_layers.first->layer;
    Neuron* output_neurons = output_layer->neurons;
    for (u32 i = 0; i < output_layer->n_neurons; ++i)
    {
        Neuron* neuron = output_neurons + i;
        // TODO: for now set the winner the first to spike
        if (neuron->spike == TRUE)
        {
            network_winner_neuron_i = i;
            network_winner_is_valid = TRUE;
            break;
        }
    }


    if (network_winner_is_valid == TRUE)
    {
        u32 actual_winner_neuron_i = inputs->inputs[0].label;
        if (network_winner_neuron_i == actual_winner_neuron_i)
        {
            reward = TRUE;
        }
        else
        {
            reward = FALSE;
        }
    }

    error:
    return reward;
}

// NOTE: R-STDP helpers
f32 _r_stdp_potentiation_learning_rule(Synapse* synapse, LearningInfo* learning_info, b32 reward)
{
    f32 dw = 0;
    SynapseLearningRSTDPExpeonential* rule = &(learning_info->r_stdp_exponential);
    if (reward)
    {
        dw = rule->reward_potentiation_factor * synapse->weight * (1 - synapse->weight);
    }
    else
    {
        dw = rule->punishment_potentiation_factor * synapse->weight * (1 - synapse->weight);
    }
    return dw;
}

f32 _r_stdp_depression_learning_rule(Synapse* synapse, LearningInfo* learning_info, b32 reward)
{
    f32 dw = 0;
    SynapseLearningRSTDPExpeonential* rule = &(learning_info->r_stdp_exponential);
    if (reward)
    {
        dw = rule->reward_depression_factor * synapse->weight * (1 - synapse->weight);
    }
    else
    {
        dw = rule->punishment_depression_factor * synapse->weight * (1 - synapse->weight);
    }
    return dw;
}