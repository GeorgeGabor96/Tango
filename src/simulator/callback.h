/* date = October 13th 2022 7:38 pm */

#ifndef CALLBACK_H
#define CALLBACK_H


typedef enum {
    CALLBACK_INVALID,
    CALLBACK_NETWORK_DUMPER,
    
} CallbackType;


typedef struct Callback {
    CallbackType type;
    
    // NOTE: use this to link callbacks between them so that we have a list
    struct Callback* next;
    
    union {
        struct {
            String* output_folder;
            // NOTE: just keep like 100 of each ouptut for each layer, or 10 or something fix
            // NOTE: when its full just dump it and start over
            // NOTE: when finished dump what we have currently
        } dumper;
    }
} Callback;


internal Callback* callback_network_dumper_create(const char* output_folder);
internal void callback_destroy(Callback* callback);
internal void callback_list_destroy(Callback* callback);

internal void callback_update(Callback* callback, Network* network);
internal void callback_run(Callback* callback, Network* network);


#endif //CALLBACK_H
