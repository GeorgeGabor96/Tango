/* date = October 13th 2022 7:38 pm */

#ifndef CALLBACK_H
#define CALLBACK_H


#include "common.h"
#include "containers/string.h"
#include "simulator/network.h"


typedef enum {
    CALLBACK_INVALID,
    CALLBACK_NETWORK_DUMPER,
    
} CallbackType;


typedef struct Callback {
    CallbackType type;
    
    union {
        struct {
            String* output_folder;
            u32 time;
            u32 sample_count;
            u32 n_layers;
            DumpLayerData* data;
        } Dumper;
    };
} Callback, *CallbackP;


internal Callback* callback_network_dumper_create(const char* output_folder);
internal void callback_destroy(Callback* callback);

internal void callback_update(Callback* callback, Network* network);
internal void callback_run(Callback* callback, Network* network);


#endif //CALLBACK_H
