internal char*
callback_type_get_c_str(CallbackType type) {
    static char* callbacks_str[CALLBACK_COUNT] = {
        "CALLBACK_META_DUMPER",
        "CALLBACK_NETWORK_DATA_DUMPER",
        "CALLBACK_SPIKES_DUMPER",
        "CALLBACK_WEIGHTS_DUMPER",
        "CALLBACK_SYNAPTIC_RESCALE",
        "CALLBACK_STDP_V1",
        "CALLBACK_ACCURACY",
        "CALLBACK_LEARNING_HISTORY",
        "CALLBACK_LEARNING_CUSTOM",
    };

    if (type >= CALLBACK_COUNT)
    {
        return "CALLBACK_INVALID";
    }

    return callbacks_str[type];
}

b8 callback_is_valid(Callback* callback)
{
    check(callback != NULL, "callback is NULL");
    if (callback->type >= CALLBACK_COUNT)
    {
        log_error("Unknown callback type %u (%s)", callback->type, callback_type_get_c_str(callback->type));
    }

    return TRUE;

    error:
    return FALSE;
}

internal CALLBACK_BEGIN_SAMPLE(callback_begin_sample)
{
    static CALLBACK_BEGIN_SAMPLE_FN* callbacks[CALLBACK_COUNT] = {
        callback_meta_dumper_begin_sample,
        callback_network_data_dumper_begin_sample,
        callback_spikes_dumper_begin_sample,
        callback_weights_dumper_begin_sample,
        callback_synaptic_rescale_begin_sample,
        callback_stdp_v1_begin_sample,
        callback_accuracy_begin_sample,
        callback_learning_history_begin_sample,
        callback_learning_custom_begin_sample,
    };

    check(callback_is_valid(callback) == TRUE, "callback is invalid");
    check(memory != NULL, "memory is NULL");
    callbacks[callback->type](callback, sample, epoch_i, memory);

    error:
    return;
}


internal CALLBACK_UPDATE(callback_update)
{
    static CALLBACK_UPDATE_FN* callbacks[CALLBACK_COUNT] = {
        callback_meta_dumper_update,
        callback_network_data_dumper_update,
        callback_spikes_dumper_update,
        callback_weights_dumper_update,
        callback_synaptic_rescale_update,
        callback_stdp_v1_update,
        callback_accuracy_update,
        callback_learning_history_update,
        callback_learning_custom_update,
    };

    check(callback_is_valid(callback) == TRUE, "callback is invalid");
    check(memory != NULL, "memory is NULL");
    callbacks[callback->type](callback, sample, inputs, time, memory);

    error:
    return;
}


internal CALLBACK_END_SAMPLE(callback_end_sample)
{
    static CALLBACK_END_SAMPLE_FN* callbacks[CALLBACK_COUNT] = {
        callback_meta_dumper_end_sample,
        callback_network_data_dumper_end_sample,
        callback_spikes_dumper_end_sample,
        callback_weights_dumper_end_sample,
        callback_synaptic_rescale_end_sample,
        callback_stdp_v1_end_sample,
        callback_accuracy_end_sample,
        callback_learning_history_end_sample,
        callback_learning_custom_end_sample,
    };

    check(callback_is_valid(callback) == TRUE, "callback is invalid");
    check(memory != NULL, "memory is NULL");
    callbacks[callback->type](callback, sample, reward, epoch_i, memory);

    error:
    return;
}


internal CALLBACK_BEGIN_EPOCH(callback_begin_epoch)
{
    static CALLBACK_BEGIN_EPOCH_FN* callbacks[CALLBACK_COUNT] = {
        callback_meta_dumper_begin_epoch,
        callback_network_data_dumper_begin_epoch,
        callback_spikes_dumper_begin_epoch,
        callback_weights_dumper_begin_epoch,
        callback_synaptic_rescale_begin_epoch,
        callback_stdp_v1_begin_epoch,
        callback_accuracy_begin_epoch,
        callback_learning_history_begin_epoch,
        callback_learning_custom_begin_epoch,
    };

    check(callback_is_valid(callback) == TRUE, "callback is invalid");
    check(memory != NULL, "memory is NULL");
    callbacks[callback->type](callback, epoch_i, memory);

    error:
    return;
}


internal CALLBACK_END_EPOCH(callback_end_epoch)
{
    static CALLBACK_END_EPOCH_FN* callbacks[CALLBACK_COUNT] = {
        callback_meta_dumper_end_epoch,
        callback_network_data_dumper_end_epoch,
        callback_spikes_dumper_end_epoch,
        callback_weights_dumper_end_epoch,
        callback_synaptic_rescale_end_epoch,
        callback_stdp_v1_end_epoch,
        callback_accuracy_end_epoch,
        callback_learning_history_end_epoch,
        callback_learning_custom_end_epoch,
    };

    check(callback_is_valid(callback) == TRUE, "callback is invalid");
    check(memory != NULL, "memory is NULL");
    callbacks[callback->type](callback, epoch_i, memory);

    error:
    return;
}


internal CALLBACK_BEGIN_EXPERIMENT(callback_begin_experiment)
{
    static CALLBACK_BEGIN_EXPERIMENT_FN* callbacks[CALLBACK_COUNT] = {
        callback_meta_dumper_begin_experiment,
        callback_network_data_dumper_begin_experiment,
        callback_spikes_dumper_begin_experiment,
        callback_weights_dumper_begin_experiment,
        callback_synaptic_rescale_begin_experiment,
        callback_stdp_v1_begin_experiment,
        callback_accuracy_begin_experiment,
        callback_learning_history_begin_experiment,
        callback_learning_custom_begin_experiment,
    };

    check(callback_is_valid(callback) == TRUE, "callback is invalid");
    callbacks[callback->type](callback);

    error:
    return;
}


internal CALLBACK_END_EXPERIMENT(callback_end_experiment)
{
    static CALLBACK_END_EXPERIMENT_FN* callbacks[CALLBACK_COUNT] = {
        callback_meta_dumper_end_experiment,
        callback_network_data_dumper_end_experiment,
        callback_spikes_dumper_end_experiment,
        callback_weights_dumper_end_experiment,
        callback_synaptic_rescale_end_experiment,
        callback_stdp_v1_end_experiment,
        callback_accuracy_end_experiment,
        callback_learning_history_end_experiment,
        callback_learning_custom_end_experiment,
    };

    check(callback_is_valid(callback) == TRUE, "callback is invalid");
    callbacks[callback->type](callback);

    error:
    return;
}
