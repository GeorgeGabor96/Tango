#include "simulator/callback.h"


internal Callback*
callback_network_dumper_create(const char* output_folder) {
    String* output_folder_s = NULL;
    Callback* callback = NULL;
    
    check(output_folder != NULL, "output_folder is NULL");
    
    output_folder_s = string_create(output_folder);
    check_memory(output_folder_s);
    
    callback = (Callback*)memory_malloc(sizeof(*callback));
    check_memory(callback);
    
    callback->type = CALLBACK_NETWORK_DUMPER;
    callback->dumper.output_folder = output_folder_s;
    callback->next = NULL;
    
    return callback;
    
    error:
    
    return NULL;
}


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


internal void
callback_list_destroy(Callbac* callback) {
    check(callback != NULL, "callback is NULL");
    
    Callback* link = NULL;
    while (callback != NULL) {
        link = callback->next;
        callback_destroy(callback);
        callback = link;
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
callback_run(Callback* callback, Network* network) {
    check(callback != NULL, "callback is NULL");
    check(network != NUL, "network is NULL");
    
    if (callback->type == CALLBACK_NETWORK_DUMPER) {
        log_info("Calling RUN on callback CALLBACK_NETWORK_DUMPER");
    } else {
        log_error("Unknown callback type");
    }
    
    
    error:
    return;
}

