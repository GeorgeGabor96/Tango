/* date = October 13th 2022 7:38 pm */

#ifndef CALLBACK_H
#define CALLBACK_H


#include "common.h"
#include "utils/memory.h"
#include "utils/os.h"
#include "containers/string.h"
#include "simulator/network.h"


/*****************************
*  NETWORK DUMPER definitions
*****************************/
// TODO: Can't I use this directly in the neuron class???
typedef struct DumperNeuronStep {
    f32 voltage;
    bool spike;
    f32 psc;
    f32 epsc;
    f32 ipsc;
} DumperNeuronStep, *DumperNeuronStepP;


typedef struct DumperLayerData {
    u32 n_neurons;
    DumperNeuronStepP* neurons_steps;
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


typedef struct Callback {
    CallbackType type;
    
    union {
        Dumper dumper;
    };
} Callback, *CallbackP;


/********************
* Callback functions
********************/
internal void callback_destroy(Callback* callback);

internal void callback_begin_sample(Callback* callback, Network* network, u32 sample_duration);
internal void callback_update(Callback* callback, Network* network);
internal void callback_end_sample(Callback* callback, Network* network);


/**************************
* Network Dumper functions
**************************/
internal Callback* callback_dumper_create(const char* output_folder, 
                                          Network* network);
internal void callback_dumper_destroy(Callback* callback);
internal void callback_dumper_begin_sample(Callback* callback, 
                                           Network* network, 
                                           u32 sample_duration);
internal void callback_dumper_update(Callback* callback, Network* network);
internal void callback_dumper_end_sample(Callback* callbac, Network* network);


#endif //CALLBACK_H
