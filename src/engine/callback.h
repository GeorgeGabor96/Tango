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
    String* output_folder;
    FILE* sample_fp;

    DumperNeuronData* neuron_data;
    DumperSynapseData* synapse_data;
} DumperData;


/************************
* Callback Spikes Dumper
************************/
typedef struct DumperSpikes {
    String* output_folder;
    FILE* sample_fp;

    NeuronTimeSpike* spikes_data;
    u32 n_spikes;
} DumperSpikes;


/************************
* Callback Weigth Dumper
************************/
typedef struct DumperWeights {
    String* output_folder;
    FILE* sample_fp;

    u32 time_step;
    u32 next_time_to_dump_i;
    u32 sample_step;
    u32 next_sample_to_dump_i;
    f32* weights;
} DumperWeights;


/****************************
* Callback Synaptic Rescale
****************************/
typedef struct SynapticRescale {
    f32 neurotransmitter_quantity;
} SynapticRescale;


/****************************
* Callback STDP V1
****************************/
typedef struct STDPv1 {
    u8* cooldown;
    u8 cooldown_value;
} STDPv1;


/****************************
 * Callback Accuracy
****************************/
typedef struct Accuracy
{
    u32 hits;
    u32 total;
    FILE* fp;
} Accuracy;


/****************************
* Callback STDP V1
****************************/
typedef struct LearningHistory {
    void* data;
} LearningHistory;


/**********************
* Callback definitions
**********************/
typedef enum {
    CALLBACK_META_DUMPER = 0,
    CALLBACK_NETWORK_DATA_DUMPER = 1,
    CALLBACK_SPIKES_DUMPER = 2,
    CALLBACK_WEIGHTS_DUMPER = 3,
    CALLBACK_SYNAPTIC_RESCALE = 4,
    CALLBACK_STDP_V1 = 5,
    CALLBACK_ACCURACY = 6,
    CALLBACK_LEARNING_HISTORY = 7,

    CALLBACK_COUNT,
    CALLBACK_INVALID,
} CallbackType;

internal char* callback_type_get_c_str(CallbackType type);


typedef struct Callback {
    CallbackType type;
    Network* network;

    union {
        DumperMeta dumper_meta;
        DumperData dumper_data;
        DumperSpikes dumper_spikes;
        DumperWeights dumper_weights;
        SynapticRescale synaptic_rescale;
        STDPv1 stdp_v1;
        Accuracy accuracy;
        LearningHistory learning_history;
    };
} Callback, *CallbackP;


/********************
* Callback functions
********************/
#define CALLBACK_BEGIN_SAMPLE(name) void name(Callback* callback, DataSample* sample, u32 epoch_i, Memory* memory)
typedef CALLBACK_BEGIN_SAMPLE(CALLBACK_BEGIN_SAMPLE_FN);
internal CALLBACK_BEGIN_SAMPLE(callback_begin_sample);

#define CALLBACK_UPDATE(name) void name(Callback* callback, DataSample* sample, Inputs* inputs, u32 time, Memory* memory)
typedef CALLBACK_UPDATE(CALLBACK_UPDATE_FN);
internal CALLBACK_UPDATE(callback_update);

#define CALLBACK_END_SAMPLE(name) void name(Callback* callback, DataSample* sample, u32 epoch_i, Memory* memory)
typedef CALLBACK_END_SAMPLE(CALLBACK_END_SAMPLE_FN);
internal CALLBACK_END_SAMPLE(callback_end_sample);

#define CALLBACK_BEGIN_EPOCH(name) void name(Callback* callback, u32 epoch_i, Memory* memory)
typedef CALLBACK_BEGIN_EPOCH(CALLBACK_BEGIN_EPOCH_FN);
internal CALLBACK_BEGIN_EPOCH(callback_begin_epoch);

#define CALLBACK_END_EPOCH(name) void name(Callback* callback, u32 epoch_i, Memory* memory)
typedef CALLBACK_END_EPOCH(CALLBACK_END_EPOCH_FN);
internal CALLBACK_END_EPOCH(callback_end_epoch);

#define CALLBACK_BEGIN_EXPERIMENT(name) void name(Callback* callback)
typedef CALLBACK_BEGIN_EXPERIMENT(CALLBACK_BEGIN_EXPERIMENT_FN);
internal CALLBACK_BEGIN_EXPERIMENT(callback_begin_experiment);

#define CALLBACK_END_EXPERIMENT(name) void name(Callback* callback)
typedef CALLBACK_END_EXPERIMENT(CALLBACK_END_EXPERIMENT_FN);
internal CALLBACK_END_EXPERIMENT(callback_end_experiment);

/**********************
* Callback Meta Dumper
**********************/
internal Callback* callback_meta_dumper_create(
    Memory* memory,
    String* output_folder,
    Network* network);

internal CALLBACK_BEGIN_SAMPLE(callback_meta_dumper_begin_sample);
internal CALLBACK_UPDATE(callback_meta_dumper_update);
internal CALLBACK_END_SAMPLE(callback_meta_dumper_end_sample);
internal CALLBACK_BEGIN_EPOCH(callback_meta_dumper_begin_epoch);
internal CALLBACK_END_EPOCH(callback_meta_dumper_end_epoch);
internal CALLBACK_BEGIN_EXPERIMENT(callback_meta_dumper_begin_experiment);
internal CALLBACK_END_EXPERIMENT(callback_meta_dumper_end_experiment);

/***********************
* Callback Data Dumper
***********************/
internal Callback* callback_network_data_dumper_create(
    Memory* memory,
    String* output_folder,
    Network* network);

internal CALLBACK_BEGIN_SAMPLE(callback_network_data_dumper_begin_sample);
internal CALLBACK_UPDATE(callback_network_data_dumper_update);
internal CALLBACK_END_SAMPLE(callback_network_data_dumper_end_sample);
internal CALLBACK_BEGIN_EPOCH(callback_network_data_dumper_begin_epoch);
internal CALLBACK_END_EPOCH(callback_network_data_dumper_end_epoch);
internal CALLBACK_BEGIN_EXPERIMENT(callback_network_data_dumper_begin_experiment);
internal CALLBACK_END_EXPERIMENT(callback_network_data_dumper_end_experiment);

/***********************
* Callback Spikes Dumper
***********************/
internal Callback* callback_spikes_dumper_create(
    Memory* memory,
    String* output_folder,
    Network* network);

internal CALLBACK_BEGIN_SAMPLE(callback_spikes_dumper_begin_sample);
internal CALLBACK_UPDATE(callback_spikes_dumper_update);
internal CALLBACK_END_SAMPLE(callback_spikes_dumper_end_sample);
internal CALLBACK_BEGIN_EPOCH(callback_spikes_dumper_begin_epoch);
internal CALLBACK_END_EPOCH(callback_spikes_dumper_end_epoch);
internal CALLBACK_BEGIN_EXPERIMENT(callback_spikes_dumper_begin_experiment);
internal CALLBACK_END_EXPERIMENT(callback_spikes_dumper_end_experiment);

/*************************
* Callback Weights Dumper
*************************/
internal Callback* callback_weights_dumper_create(
    Memory* memory,
    u32 sample_step,
    u32 time_step,
    String* output_folder,
    Network* network);

internal CALLBACK_BEGIN_SAMPLE(callback_weights_dumper_begin_sample);
internal CALLBACK_UPDATE(callback_weights_dumper_update);
internal CALLBACK_END_SAMPLE(callback_weights_dumper_end_sample);
internal CALLBACK_BEGIN_EPOCH(callback_weights_dumper_begin_epoch);
internal CALLBACK_END_EPOCH(callback_weights_dumper_end_epoch);
internal CALLBACK_BEGIN_EXPERIMENT(callback_weights_dumper_begin_experiment);
internal CALLBACK_END_EXPERIMENT(callback_weights_dumper_end_experiment);

/*************************
* Callback Synaptic Rescale
*************************/
internal Callback* callback_synaptic_rescale_create(
    Memory* memory,
    Network* network,
    f32 neurotransmitter_quantity);

internal CALLBACK_BEGIN_SAMPLE(callback_synaptic_rescale_begin_sample);
internal CALLBACK_UPDATE(callback_synaptic_rescale_update);
internal CALLBACK_END_SAMPLE(callback_synaptic_rescale_end_sample);
internal CALLBACK_BEGIN_EPOCH(callback_synaptic_rescale_begin_epoch);
internal CALLBACK_END_EPOCH(callback_synaptic_rescale_end_epoch);
internal CALLBACK_BEGIN_EXPERIMENT(callback_synaptic_rescale_begin_experiment);
internal CALLBACK_END_EXPERIMENT(callback_synaptic_rescale_end_experiment);

/*************************
* Callback STDP V1
*************************/
internal Callback* callback_stdp_v1_create(
    Memory* memory,
    Network* network,
    u8 cooldown_value);

internal CALLBACK_BEGIN_SAMPLE(callback_stdp_v1_begin_sample);
internal CALLBACK_UPDATE(callback_stdp_v1_update);
internal CALLBACK_END_SAMPLE(callback_stdp_v1_end_sample);
internal CALLBACK_BEGIN_EPOCH(callback_stdp_v1_begin_epoch);
internal CALLBACK_END_EPOCH(callback_stdp_v1_end_epoch);
internal CALLBACK_BEGIN_EXPERIMENT(callback_stdp_v1_begin_experiment);
internal CALLBACK_END_EXPERIMENT(callback_stdp_v1_end_experiment);

/*************************
* Callback Accuracy
*************************/
internal Callback* callback_accuracy_create(
    Memory* memory,
    String* output_folder,
    Network* network);

internal CALLBACK_BEGIN_SAMPLE(callback_accuracy_begin_sample);
internal CALLBACK_UPDATE(callback_accuracy_update);
internal CALLBACK_END_SAMPLE(callback_accuracy_end_sample);
internal CALLBACK_BEGIN_EPOCH(callback_accuracy_begin_epoch);
internal CALLBACK_END_EPOCH(callback_accuracy_end_epoch);
internal CALLBACK_BEGIN_EXPERIMENT(callback_accuracy_begin_experiment);
internal CALLBACK_END_EXPERIMENT(callback_accuracy_end_experiment);


/*************************
* Callback Learning History
*************************/
internal Callback* callback_learning_history_create(
    Memory* memory,
    Network* network);

internal CALLBACK_BEGIN_SAMPLE(callback_learning_history_begin_sample);
internal CALLBACK_UPDATE(callback_learning_history_update);
internal CALLBACK_END_SAMPLE(callback_learning_history_end_sample);
internal CALLBACK_BEGIN_EPOCH(callback_learning_history_begin_epoch);
internal CALLBACK_END_EPOCH(callback_learning_history_end_epoch);
internal CALLBACK_BEGIN_EXPERIMENT(callback_learning_history_begin_experiment);
internal CALLBACK_END_EXPERIMENT(callback_learning_history_end_experiment);


#endif // __ENGINE_CALLBACK_H__
