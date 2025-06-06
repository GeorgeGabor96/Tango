internal Callback*
callback_stdp_v1_create(Memory* memory, Network* network, u8 cooldown_value)
{
    check(memory != NULL, "memory is NULL");
    check(network != NULL, "network is NULL");

    Callback* callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    b8* cooldown = (b8*)memory_push_zero(memory, sizeof(b8) * network->n_synapses);
    check_memory(cooldown);

    callback->type = CALLBACK_STDP_V1;
    callback->network = network;
    callback->stdp_v1.cooldown = cooldown;
    callback->stdp_v1.cooldown_value = cooldown_value;

    return callback;

    error:
    return NULL;
}


internal CALLBACK_BEGIN_SAMPLE(callback_stdp_v1_begin_sample)
{

}

static b8 _callback_stdp_v1_synapse_update(Synapse* synapse, DataSample* sample, Inputs* inputs, Network* network);


internal CALLBACK_UPDATE(callback_stdp_v1_update)
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
        b8 did_change = _callback_stdp_v1_synapse_update(synapse, sample, inputs, callback->network);
        if (did_change)
        {
            data->cooldown[i] = data->cooldown_value;
        }
    }
}

f32 _r_stdp_dw_potentiation_learning_rule(Synapse* synapse, LearningInfo* learning_info, b32 reward);
f32 _r_stdp_dw_depression_learning_rule(Synapse* synapse, LearningInfo* learning_info, b32 reward);

f32 _r_stdp_exponential_potentiation_learning_rule(Synapse* synapse, LearningInfo* learning_info, b32 reward, u32 dt);
f32 _r_stdp_exponential_depression_learning_rule(Synapse* synapse, LearningInfo* learning_info, b32 reward, u32 dt);

static b8
_callback_stdp_v1_synapse_update(Synapse* synapse, DataSample* sample, Inputs* inputs, Network* network)
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

    b32 reward = callback_utils_get_reward_first_spike(network, sample);

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
        else if (learning_info->type == SYNAPSE_LEARNING_RSTDP_DW)
        {
            dw = _r_stdp_dw_potentiation_learning_rule(synapse, learning_info, reward);
        }
        else if (learning_info->type == SYNAPSE_LEARNING_RSTDP_EXPONENTIAL)
        {
            dw = _r_stdp_exponential_potentiation_learning_rule(synapse, learning_info, reward, dt);
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
        else if (learning_info->type == SYNAPSE_LEARNING_RSTDP_DW)
        {
            dw = _r_stdp_dw_depression_learning_rule(synapse, learning_info, reward);
        }
        else if (learning_info->type == SYNAPSE_LEARNING_RSTDP_EXPONENTIAL)
        {
            dw = _r_stdp_exponential_depression_learning_rule(synapse, learning_info, reward, dt);
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


internal CALLBACK_END_SAMPLE(callback_stdp_v1_end_sample)
{

}

// NOTE: R-STDP helpers
f32 _r_stdp_dw_potentiation_learning_rule(Synapse* synapse, LearningInfo* learning_info, b32 reward)
{
    f32 dw = 0;
    SynapseLearningRSTDPdw* rule = &(learning_info->r_stdp_dw);
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

f32 _r_stdp_dw_depression_learning_rule(Synapse* synapse, LearningInfo* learning_info, b32 reward)
{
    f32 dw = 0;
    SynapseLearningRSTDPdw* rule = &(learning_info->r_stdp_dw);
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

f32 _r_stdp_exponential_potentiation_learning_rule(Synapse* synapse, LearningInfo* learning_info, b32 reward, u32 dt)
{
    f32 dw = 0;
    SynapseLearningRSTDPexponential* rule = &(learning_info->r_stdp_exponential);
    if (reward)
    {
        dw = rule->A * math_exp_f32(-(f32)dt / rule->tau);
    }
    else
    {
        dw = rule->B * math_exp_f32(-(f32)dt / rule->tau);
    }
    return dw;
}

f32 _r_stdp_exponential_depression_learning_rule(Synapse* synapse, LearningInfo* learning_info, b32 reward, u32 dt)
{
    f32 dw = 0;
    SynapseLearningRSTDPexponential* rule = &(learning_info->r_stdp_exponential);
    if (reward)
    {
        dw = rule->B * math_exp_f32(-(f32)dt / rule->tau);
    }
    else
    {
        dw = rule->A * math_exp_f32(-(f32)dt / rule->tau);
    }
    return dw;
}


internal CALLBACK_BEGIN_EPOCH(callback_stdp_v1_begin_epoch)
{

}


internal CALLBACK_END_EPOCH(callback_stdp_v1_end_epoch)
{

}


internal CALLBACK_BEGIN_EXPERIMENT(callback_stdp_v1_begin_experiment)
{

}


internal CALLBACK_END_EXPERIMENT(callback_stdp_v1_end_experiment)
{

}