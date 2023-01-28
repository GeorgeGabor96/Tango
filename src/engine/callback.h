#ifndef __ENGINE_CALLBACK_H__
#define __ENGINE_CALLBACK_H__


/*****************************
*  NETWORK DUMPER definitions
*****************************/
typedef struct DumperLayerMeta {
    String* name;
    u32 neuron_start_i;
    u32 n_neurons;
} DumperLayerMeta;

typedef struct DumperSynapseMeta {
    u32 in_neuron_i;
    u32 out_neuron_i;
} DumperSynapseMeta;

typedef struct DumperMeta {
    String* file_path;

    u32 n_layers;
    u32 n_neurons;
    u32 n_synapses;

    DumperLayerMeta* layer_meta;
    DumperSynapseMeta* synapse_meta;
} DumperMeta;


typedef struct DumperNeuronData {
    f32 voltage;
    bool spike;
    f32 psc;
    f32 epsc;
    f32 ipsc;
} DumperNeuronData;

typedef struct DumperSynapseData {
    f32 weight;
    f32 conductance;
} DumperSynapseData;

typedef struct DumperData {
    String* file_name;
    FILE* fp;

    DumperNeuronData* neuron_data;
    DumperSynapseData* synapse_data;
} DumperData;


typedef struct Dumper {
    String* output_folder;

    u32 sample_step;
    u32 sample_count;
    u32 sample_duration;

    DumperMeta* meta;
    DumperData* data;
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


#endif // __ENGINE_CALLBACK_H__
