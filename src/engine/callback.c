#include "engine/callbacks/dumper.c"


internal char*
callback_type_get_c_str(CallbackType type) {
    if (type == CALLBACK_NETWORK_DUMPER) return "CALLBACK_NETWORK_DUMPER";
    return "CALLBACK_INVALID";
}


internal void
callback_begin_sample(Callback* callback,
                      Network* network,
                      u32 sample_duration,
                      Memory* memory) {
    check(callback != NULL, "callback is NULL");
    check(network != NULL, "network is NULL");
    check(sample_duration != 0, "sample_duration is NULL");
    check(memory != NULL, "memory is NULL");

    if (callback->type == CALLBACK_NETWORK_DUMPER) {
        callback_dumper_begin_sample(callback, network, sample_duration, memory);
    } else {
        log_error("Unknown callback type %u (%s)",
                  callback->type,
                  callback_type_get_c_str(callback->type));
    }

    error:
    return;
}


internal void
callback_update(Callback* callback, Network* network, Memory* memory) {
    check(callback != NULL, "callback is NULL");
    check(network != NULL, "network is NULL");
    check(memory != NULL, "memory is NULL");

    if (callback->type == CALLBACK_NETWORK_DUMPER) {
        callback_dumper_update(callback, network, memory);
    } else {
        log_error("Unknown callback type %u (%s)",
                  callback->type,
                  callback_type_get_c_str(callback->type));
    }

    error:
    return;
}


internal void
callback_end_sample(Callback* callback, Network* network, Memory* memory) {
    check(callback != NULL, "callback is NULL");
    check(network != NULL, "network is NULL");
    check(memory != NULL, "memory is NULL");

    if (callback->type == CALLBACK_NETWORK_DUMPER) {
        callback_dumper_end_sample(callback, network, memory);
    } else {
        log_error("Unknown callback type %u (%s)",
                  callback->type,
                  callback_type_get_c_str(callback->type));
    }

    error:
    return;
}
