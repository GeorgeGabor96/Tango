internal void _callback_learning_history_update_synapse(Synapse* synapse, Callback* callback, DataSample* sample, b32 reward);

#define CALLBACK_LEARNING_HISTORY_COMPUTE_DW(name) f32 name(Synapse* synapse, u32 pre_spike_time, u32 post_spike_time, Network* network, DataSample* sample, b32 reward)
typedef CALLBACK_LEARNING_HISTORY_COMPUTE_DW(CALLBACK_LEARNING_HISTORY_COMPUTE_DW_FN);
internal CALLBACK_LEARNING_HISTORY_COMPUTE_DW(_callback_learning_history_compute_dw_no_learning);
internal CALLBACK_LEARNING_HISTORY_COMPUTE_DW(_callback_learning_history_compute_dw_learning_exponential);
internal CALLBACK_LEARNING_HISTORY_COMPUTE_DW(_callback_learning_history_compute_dw_learning_step);
internal CALLBACK_LEARNING_HISTORY_COMPUTE_DW(_callback_learning_history_compute_dw_learning_rstdp_dw);
internal CALLBACK_LEARNING_HISTORY_COMPUTE_DW(_callback_learning_history_compute_dw_learning_rstdp_exponential);

internal b8
_callback_learning_history_compute_dw_learning_reward_modulated_get_reward(Network* network, DataSample* sample);

internal Callback*
callback_learning_history_create(Memory* memory, Network* network)
{
    check(memory != NULL, "memory is NULL");
    check(network != NULL, "network is NULL");

    Callback* callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    callback->type = CALLBACK_LEARNING_HISTORY;
    callback->network = network;
    callback->learning_history.data = NULL;

    return callback;

    error:
    return NULL;
}

internal CALLBACK_BEGIN_SAMPLE(callback_learning_history_begin_sample)
{

}

internal CALLBACK_UPDATE(callback_learning_history_update)
{

}

internal CALLBACK_END_SAMPLE(callback_learning_history_end_sample)
{
    Network* network = callback->network;

    for (u32 i = 0; i < network->n_synapses; ++i)
    {
        Synapse* synapse = network->synapses + i;
        _callback_learning_history_update_synapse(synapse, callback, sample, reward);
    }
}

internal CALLBACK_BEGIN_EPOCH(callback_learning_history_begin_epoch)
{

}

internal CALLBACK_END_EPOCH(callback_learning_history_end_epoch)
{

}

internal CALLBACK_BEGIN_EXPERIMENT(callback_learning_history_begin_experiment)
{

}

internal CALLBACK_END_EXPERIMENT(callback_learning_history_end_experiment)
{

}


internal void
_callback_learning_history_update_synapse(Synapse* synapse, Callback* callback, DataSample* sample, b32 reward)
{
    CALLBACK_LEARNING_HISTORY_COMPUTE_DW_FN* compute_dw_fn[SYNAPSE_LEARNING_COUNT] =
    {
        _callback_learning_history_compute_dw_no_learning,
        _callback_learning_history_compute_dw_learning_exponential,
        _callback_learning_history_compute_dw_learning_step,
        _callback_learning_history_compute_dw_learning_rstdp_dw,
        _callback_learning_history_compute_dw_learning_rstdp_exponential,
    };

    f32 dw = 0.0f;
    SynapseCls* cls = synapse->cls;
    LearningInfo* learning_info = &(cls->learning_info);
    if (learning_info->type > SYNAPSE_LEARNING_COUNT)
    {
        log_error("Unkown Synapse Learning Rule %s (%u)",
            synapse_learning_rule_get_c_str(learning_info->type),
            learning_info->type);
        return;
    }

    if (learning_info->enable == FALSE) return;

    Neuron* in_neuron = synapse->in_neuron;
    Neuron* out_neuron = synapse->out_neuron;

    for (u32 in_neuron_spike_i = 0; in_neuron_spike_i < in_neuron->n_spikes; ++in_neuron_spike_i)
    {
        for (u32 out_neuron_spike_i = 0; out_neuron_spike_i < out_neuron->n_spikes; ++out_neuron_spike_i)
        {
            u32 pre_spike_time = in_neuron->spike_times[in_neuron_spike_i];
            u32 post_spike_time = out_neuron->spike_times[out_neuron_spike_i];

            f32 partial_dw = compute_dw_fn[learning_info->type](synapse, pre_spike_time, post_spike_time, callback->network, sample, reward);
            dw += partial_dw;
        }
    }

    if (dw != 0)
    {
        synapse->weight = math_clip_f32(synapse->weight + dw, learning_info->min_w, learning_info->max_w);
    }
    return;
}

internal CALLBACK_LEARNING_HISTORY_COMPUTE_DW(_callback_learning_history_compute_dw_no_learning)
{
    return 0.0f;
}

internal CALLBACK_LEARNING_HISTORY_COMPUTE_DW(_callback_learning_history_compute_dw_learning_exponential)
{
    f32 dw = 0.0f;
    SynapseLearningExponential* rule = &(synapse->cls->learning_info.stdp_exponential);
    if (pre_spike_time < post_spike_time)
    {
        u32 dt = post_spike_time - pre_spike_time;
        dw = rule->A * math_exp_f32(-(f32)dt / rule->tau);
    }
    else
    {
        u32 dt = pre_spike_time - post_spike_time;
        dw = rule->B * math_exp_f32(-(f32)dt / rule->tau);
    }
    return dw;
}

internal CALLBACK_LEARNING_HISTORY_COMPUTE_DW(_callback_learning_history_compute_dw_learning_step)
{
    f32 dw = 0.0f;
    SynapseLearningStep* rule = &(synapse->cls->learning_info.stdp_step);
    if (pre_spike_time < post_spike_time)
    {
        u32 dt = post_spike_time - pre_spike_time;
        if (dt <= rule->max_time_p) dw = rule->amp_p;
    }
    else
    {
        u32 dt = pre_spike_time - post_spike_time;
        if (dt <= rule->max_time_d) dw = rule->amp_d;
    }
    return dw;
}

internal CALLBACK_LEARNING_HISTORY_COMPUTE_DW(_callback_learning_history_compute_dw_learning_rstdp_dw)
{
    f32 dw = 0.0f;
    SynapseLearningRSTDPdw* rule = &(synapse->cls->learning_info.r_stdp_dw);

    if (pre_spike_time < post_spike_time)
    {
        if (reward == TRUE)
        {
            dw = rule->reward_potentiation_factor * synapse->weight * (1 - synapse->weight);
        }
        else
        {
            dw = rule->punishment_potentiation_factor * synapse->weight * (1 - synapse->weight);
        }
    }
    else
    {
        if (reward == TRUE)
        {
            dw = rule->reward_depression_factor * synapse->weight * (1 - synapse->weight);
        }
        else
        {
            dw = rule->punishment_depression_factor * synapse->weight * (1 - synapse->weight);
        }
    }
    return dw;
}

internal CALLBACK_LEARNING_HISTORY_COMPUTE_DW(_callback_learning_history_compute_dw_learning_rstdp_exponential)
{
    f32 dw = 0.0f;
    SynapseLearningExponential* rule = &(synapse->cls->learning_info.stdp_exponential);
    if (pre_spike_time < post_spike_time)
    {
        u32 dt = post_spike_time - pre_spike_time;
        if (dt <= 20)
        {
            if (reward == TRUE)
            {
                dw = rule->A * math_exp_f32(-(f32)dt / rule->tau);
            }
            else
            {
                dw = rule->B * math_exp_f32(-(f32)dt / rule->tau);
            }
        }

    }
    else
    {
        u32 dt = pre_spike_time - post_spike_time;
        if (dt <= 20)
        {
            if (reward == TRUE)
            {
                dw = rule->B * math_exp_f32(-(f32)dt / rule->tau);
            }
            else
            {
                dw = rule->A * math_exp_f32(-(f32)dt / rule->tau);
            }
        }

    }
    return dw;
}

