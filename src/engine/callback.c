internal char*
callback_type_get_c_str(CallbackType type) {
    if (type == CALLBACK_META_DUMPER)
        return "CALLBACK_META_DUMPER";
    if (type == CALLBACK_NETWORK_DATA_DUMPER)
        return "CALLBACK_NETWORK_DATA_DUMPER";
    if (type == CALLBACK_SPIKES_DUMPER)
        return "CALLBACK_SPIKES_DUMPER";
    if (type == CALLBACK_WEIGHTS_DUMPER)
        return "CALLBACK_WEIGHTS_DUMPER";
    if (type == CALLBACK_SYNAPTIC_RESCALE)
        return "CALLBACK_SYNAPTIC_RESCALE";
    return "CALLBACK_INVALID";
}


internal void
callback_begin_sample(Callback* callback,
                      DataSample* sample,
                      Memory* memory) {
    check(callback != NULL, "callback is NULL");
    check(sample != 0, "sample is NULL");
    check(memory != NULL, "memory is NULL");

    if (callback->type == CALLBACK_META_DUMPER) {
        callback_meta_dumper_begin_sample(callback, sample, memory);
    } else if (callback->type == CALLBACK_NETWORK_DATA_DUMPER) {
        callback_network_data_dumper_begin_sample(callback, sample, memory);
    } else if (callback->type == CALLBACK_SPIKES_DUMPER) {
        callback_spikes_dumper_begin_sample(callback, sample, memory);
    } else if (callback->type == CALLBACK_WEIGHTS_DUMPER) {
        callback_weights_dumper_begin_sample(callback, sample, memory);
    } else if (callback->type == CALLBACK_SYNAPTIC_RESCALE) {
        callback_synaptic_rescale_begin_sample(callback, sample, memory);
    } else {
        log_error("Unknown callback type %u (%s)",
                  callback->type,
                  callback_type_get_c_str(callback->type));
    }

    error:
    return;
}


internal void
callback_update(Callback* callback, u32 time, Memory* memory) {
    check(callback != NULL, "callback is NULL");
    check(memory != NULL, "memory is NULL");

    if (callback->type == CALLBACK_META_DUMPER) {
        callback_meta_dumper_update(callback, time, memory);
    } else if (callback->type == CALLBACK_NETWORK_DATA_DUMPER) {
        callback_network_data_dumper_update(callback, time, memory);
    } else if (callback->type == CALLBACK_SPIKES_DUMPER) {
        callback_spikes_dumper_update(callback, time, memory);
    } else if (callback->type == CALLBACK_WEIGHTS_DUMPER) {
        callback_weights_dumper_update(callback, time, memory);
    } else if (callback->type == CALLBACK_SYNAPTIC_RESCALE) {
        callback_synaptic_rescale_update(callback, time, memory);
    } else {
        log_error("Unknown callback type %u (%s)",
                  callback->type,
                  callback_type_get_c_str(callback->type));
    }

    error:
    return;
}


internal void
callback_end_sample(Callback* callback, Memory* memory) {
    check(callback != NULL, "callback is NULL");
    check(memory != NULL, "memory is NULL");

    if (callback->type == CALLBACK_META_DUMPER) {
        callback_meta_dumper_end_sample(callback, memory);
    } else if (callback->type == CALLBACK_NETWORK_DATA_DUMPER) {
        callback_network_data_dumper_end_sample(callback, memory);
    } else if (callback->type == CALLBACK_SPIKES_DUMPER) {
        callback_spikes_dumper_end_sample(callback, memory);
    } else if (callback->type == CALLBACK_WEIGHTS_DUMPER) {
        callback_weights_dumper_end_sample(callback, memory);
    } else if (callback->type == CALLBACK_SYNAPTIC_RESCALE) {
        callback_synaptic_rescale_end_sample(callback, memory);
    } else {
        log_error("Unknown callback type %u (%s)",
                  callback->type,
                  callback_type_get_c_str(callback->type));
    }

    error:
    return;
}
