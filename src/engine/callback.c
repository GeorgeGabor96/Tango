#include "engine/callbacks/dumper.c"


internal char*
callback_type_get_c_str(CallbackType type) {
    if (type == CALLBACK_NETWORK_DUMPER) return "CALLBACK_NETWORK_DUMPER";
    return "CALLBACK_INVALID";
}


internal void
callback_begin_sample(State* state,
                      Callback* callback,
                      Network* network,
                      u32 sample_duration) {
    check(state != NULL, "state is NULL");
    check(callback != NULL, "callback is NULL");
    check(network != NULL, "network is NULL");
    check(sample_duration != 0, "sample_duration is NULL");
    
    if (callback->type == CALLBACK_NETWORK_DUMPER) {
        callback_dumper_begin_sample(state, callback, network, sample_duration);
    } else {
        log_error("Unknown callback type %u (%s)",
                  callback->type,
                  callback_type_get_c_str(callback->type));
    }
    
    error:
    return;
}


internal void
callback_update(State* state, Callback* callback, Network* network) {
    check(state != NULL, "state is NULL");
    check(callback != NULL, "callback is NULL");
    check(network != NULL, "network is NULL");
    
    if (callback->type == CALLBACK_NETWORK_DUMPER) {
        callback_dumper_update(state, callback, network);
    } else {
        log_error("Unknown callback type %u (%s)",
                  callback->type,
                  callback_type_get_c_str(callback->type));
    }
    
    error:
    return;
}


internal void
callback_end_sample(State* state, Callback* callback, Network* network) {
    check(state != NULL, "state si NULL");
    check(callback != NULL, "callback is NULL");
    check(network != NULL, "network is NULL");
    
    if (callback->type == CALLBACK_NETWORK_DUMPER) {
        callback_dumper_end_sample(state, callback, network);
    } else {
        log_error("Unknown callback type %u (%s)",
                  callback->type,
                  callback_type_get_c_str(callback->type));
    }
    
    error:
    return;
}
