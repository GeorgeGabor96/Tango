#include "simulator/callback.h"


internal void
callback_destroy(Callback* callback) {
    check(callback != NULL, "callback is NULL");
    
    if (callback->type == CALLBACK_NETWORK_DUMPER) {
        string_destroy(callback->dumper.output_folder);
    } else {
        log_error("Unknown callback type");
    }
    
    memset(callback, 0, sizeof(*callback));
    memory_free(callback);
    
    error:
    return;
}


// TODO: maybe we need to rename the callbacks operations
internal void
callback_begin_sample(Callback* callback,
                      Network* network,
                      u32 sample_duration) {
    check(callback != NULL, "callback is NULL");
    check(network != NULL, "network is NULL");
    check(sample_duration != 0, "sample_duration is NULL");
    
    if (callback->type == CALLBACK_NETWORK_DUMPER) {
        callback_dumper_begin_sample(callback, network, duration);
    }
    
    error:
    return;
}


internal void
callback_update(Callback* callback, Network* network) {
    check(callback != NULL, "callback is NULL");
    check(network != NULL, "network is NULL");
    
    if (callback->type == CALLBACK_NETWORK_DUMPER) {
        log_info("Calling UPDATE on callback CALLBACK_NETWORK_DUMPER");
    } else {
        log_error("Unknown callback type");
    }
    
    error:
    return;
}


internal void
callback_end_sample(Callback* callback, Network* network) {
    check(callback != NULL, "callback is NULL");
    check(network != NULL, "network is NULL");
    
    if (callback->type == CALLBACK_NETWORK_DUMPER) {
        log_info("Calling RUN on callback CALLBACK_NETWORK_DUMPER");
    } else {
        log_error("Unknown callback type");
    }
    
    
    error:
    return;
}

