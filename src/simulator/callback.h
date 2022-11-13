/* date = October 13th 2022 7:38 pm */

#ifndef CALLBACK_H
#define CALLBACK_H


#include "common.h"
#include "utils/memory.h"
#include "utils/os.h"
#include "containers/string.h"
#include "containers/memory_arena.h"
#include "simulator/network.h"


/*****************************
*  NETWORK DUMPER definitions
*****************************/
typedef struct DumperNeuronData {
    f32 voltage;
    bool spike;
    f32 psc;
    f32 epsc;
    f32 ipsc;
} DumperNeuronData;


typedef struct DumperLayerData {
    String* name; // no ownership
    u32 n_neurons;
    DumperNeuronData* neurons_data;
} DumperLayerData;


typedef struct Dumper {
    String* output_folder;
    u32 time;
    u32 sample_count;
    u32 sample_duration;
    u32 n_layers;
    DumperLayerData* layers_data;
} Dumper;



/**********************
* Callback definitions
**********************/
typedef enum {
    CALLBACK_INVALID,
    CALLBACK_NETWORK_DUMPER,
} CallbackType;

internal char* callback_type_get_c_str(CallbackType type);


typedef struct Callback {
    CallbackType type;
    
    union {
        Dumper dumper;
    };
} Callback, *CallbackP;


/********************
* Callback functions
********************/
internal void callback_begin_sample(State* state, 
                                    Callback* callback,
                                    Network* network,
                                    u32 sample_duration);
internal void callback_update(State* state, Callback* callback, Network* network);
internal void callback_end_sample(State* state, Callback* callback, Network* network);


/**************************
* Network Dumper functions
**************************/
internal Callback* callback_dumper_create(State* state,
                                          const char* output_folder, 
                                          Network* network);
internal void callback_dumper_begin_sample(State* state, 
                                           Callback* callback, 
                                           Network* network, 
                                           u32 sample_duration);
internal void callback_dumper_update(State* state, Callback* callback, Network* network);
internal void callback_dumper_end_sample(State* state, Callback* callbac, Network* network);


#endif //CALLBACK_H
