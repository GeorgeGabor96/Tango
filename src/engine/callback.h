#ifndef __ENGINE_CALLBACK_H__
#define __ENGINE_CALLBACK_H__


/**********************
* Callback Meta Dumper
**********************/
typedef struct DumperMeta {
    String* meta_file;
    String* sample_name;
    u32 sample_duration;
    u32 sample_time;
} DumperMeta;


/**********************
* Callback Data Dumper
**********************/
typedef struct DumperNeuronData {
    f32 voltage;
    b32 spike;
    f32 psc;
    f32 epsc;
    f32 ipsc;
} DumperNeuronData;

typedef struct DumperSynapseData {
    f32 weight;
    f32 conductance;
} DumperSynapseData;

typedef struct DataDumper {
    Network* network;
    String* output_folder;
    FILE* sample_fp;

    DumperNeuronData* neuron_data;
    DumperSynapseData* synapse_data;
} DumperData;


/************************
* Callback Spikes Dumper
************************/
typedef struct DumperSpikes {
    Network* network;
    String* output_folder;
    FILE* sample_fp;

    u32 time;
    NeuronTimeSpike* spikes_data;
    u32 n_spikes;
} DumperSpikes;


/************************
* Callback Weigth Dumper
************************/
typedef struct DumperWeights {
    Network* network;
    String* output_folder;
    FILE* sample_fp;

    float* weights;
} DumperWeights;


/**********************
* Callback definitions
**********************/
typedef enum {
    CALLBACK_META_DUMPER = 0,
    CALLBACK_NETWORK_DATA_DUMPER = 1,
    CALLBACK_SPIKES_DUMPER = 2,
    CALLBACK_WEIGHTS_DUMPER = 3,

    CALLBACK_INVALID,
} CallbackType;

internal char* callback_type_get_c_str(CallbackType type);


typedef struct Callback {
    CallbackType type;

    union {
        DumperMeta dumper_meta;
        DumperData dumper_data;
        DumperSpikes dumper_spikes;
        DumperWeights dumper_weights;
    };
} Callback, *CallbackP;


/********************
* Callback functions
********************/
internal void callback_begin_sample(
    Callback* callback,
    DataSample* sample,
    Memory* memory);

internal void callback_update(
    Callback* callback,
    Memory* memory);

internal void callback_end_sample(
    Callback* callback,
    Memory* memory);


/**********************
* Callback Meta Dumper
**********************/
internal Callback* callback_meta_dumper_create(
    Memory* memory,
    String* output_folder,
    Network* network);


internal void callback_meta_dumper_begin_sample(
    Callback* callback,
    DataSample* sample,
    Memory* memory);


internal void callback_meta_dumper_update(
    Callback* callback,
    Memory* memory);


internal void callback_meta_dumper_end_sample(
    Callback* callback,
    Memory* memory);


/***********************
* Callback Data Dumper
***********************/
internal Callback* callback_netowrk_data_dumper_create(
    Memory* memory,
    String* output_folder,
    Network* network);


internal void callback_network_data_dumper_begin_sample(
    Callback* callback,
    DataSample* sample,
    Memory* memory);


internal void callback_network_data_dumper_update(
    Callback* callback,
    Memory* memory);


internal void callback_network_data_dumper_end_sample(
    Callback* callback,
    Memory* memory);


/***********************
* Callback Spikes Dumper
***********************/
internal Callback* callback_spikes_dumper_create(
    Memory* memory,
    String* output_folder,
    Network* network);


internal void callback_spikes_dumper_begin_sample(
    Callback* callback,
    DataSample* sample,
    Memory* memory);


internal void callback_spikes_dumper_update(
    Callback* callback,
    Memory* memory);


internal void callback_spikes_dumper_end_sample(
    Callback* callback,
    Memory* memory);


/*************************
* Callback Weights Dumper
*************************/
internal Callback* callback_weights_dumper_create(
    Memory* memory,
    String* output_folder,
    Network* network);


internal void callback_weights_dumper_begin_sample(
    Callback* callback,
    DataSample* sample,
    Memory* memory);


internal void callback_weights_dumper_update(
    Callback* callback,
    Memory* memory);


internal void callback_weights_dumper_end_sample(
    Callback* callback,
    Memory* memory);


#endif // __ENGINE_CALLBACK_H__
