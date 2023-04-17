internal char*
callback_type_get_c_str(CallbackType type) {
    if (type == CALLBACK_META_DUMPER)
        return "CALLBACK_META_DUMPER";
    if (type == CALLBACK_NETWORK_DATA_DUMPER)
        return "CALLBACK_NETWORK_DATA_DUMPER";
    if (type == CALLBACK_SPIKES_DUMPER)
        return "CALLBACK_SPIKES_DUMPER";
    return "CALLBACK_INVALID";
}


internal void
callback_begin_sample(Callback* callback,
                      u32 sample_duration,
                      Memory* memory) {
    check(callback != NULL, "callback is NULL");
    check(sample_duration != 0, "sample_duration is NULL");
    check(memory != NULL, "memory is NULL");

    if (callback->type == CALLBACK_META_DUMPER) {
        callback_meta_dumper_begin_sample(
            callback, sample_duration, memory);
    } else if (callback->type == CALLBACK_NETWORK_DATA_DUMPER) {
        callback_network_data_dumper_begin_sample(
            callback, sample_duration, memory);
    } else if (callback->type == CALLBACK_SPIKES_DUMPER) {
        callback_spikes_dumper_begin_sample(
            callback, sample_duration, memory);
    } else {
        log_error("Unknown callback type %u (%s)",
                  callback->type,
                  callback_type_get_c_str(callback->type));
    }

    error:
    return;
}


internal void
callback_update(Callback* callback, Memory* memory) {
    check(callback != NULL, "callback is NULL");
    check(memory != NULL, "memory is NULL");

    if (callback->type == CALLBACK_META_DUMPER) {
        callback_meta_dumper_update(callback, memory);
    } else if (callback->type == CALLBACK_NETWORK_DATA_DUMPER) {
        callback_network_data_dumper_update(callback, memory);
    } else if (callback->type == CALLBACK_SPIKES_DUMPER) {
        callback_spikes_dumper_update(callback, memory);
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
    } else {
        log_error("Unknown callback type %u (%s)",
                  callback->type,
                  callback_type_get_c_str(callback->type));
    }

    error:
    return;
}
