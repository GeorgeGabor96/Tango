#ifndef __ENGINE_DATA_GEN_H__
#define __ENGINE_DATA_GEN_H__


typedef enum {
    DATA_GEN_INVALID,
    DATA_GEN_CONSTANT_CURRENT,
    DATA_GEN_RANDOM_SPIKES,
    DATA_GEN_SPIKE_PULSES,
    DATA_GEN_SPIKE_TRAIN,
    DATA_GEN_BASIC_EXPERIMENT,
    DATA_GEN_BACKGROUND_ACTIVITY,
} DataGenType;


typedef struct DataGenConstCurrent {
    f32 value;
} DataGenConstCurrent;

typedef struct DataGenRandomSpikes {
    Random* random;
    f32 chance;
} DataGenRandomSpikes;

typedef struct DataGenSpikePulses {
    Random* random;
    u32 first_pulse_time;
    u32 pulse_duration;
    u32 between_pulses_duration;
    f32 pulse_spike_chance;
    f32 between_pulses_spike_chance;
} DataGenSpikePulses;

typedef struct DataGenSpikeTrain {
    StringNode* first_file_name;
    StringNode* current_sample;
    String* encodings_path;
    u32 max_time_to_use_from_train;
} DataGenSpikeTrain;

typedef struct DataGenBasicExperiment {
    Random* random;
    f32 spike_chance;
} DataGenBasicExperiment;

typedef struct DataGenBackgroundActivity
{
    Random* random;
    f32 chance_input;
    f32 change_background;
} DataGenBackgroundActivity;

typedef struct DataGen {
    DataGenType type;
    u32 n_samples;
    u32 sample_i;
    // NOTE: This may be specific per type but for now its good here
    u32 sample_duration;

    union {
        DataGenConstCurrent const_current;
        DataGenRandomSpikes random_spikes;
        DataGenSpikePulses spike_pulses;
        DataGenSpikeTrain spike_train;
        DataGenBasicExperiment basic_exp;
        DataGenBackgroundActivity background_activity;
    };
} DataGen;


internal DataGen* data_gen_create_constant_current(Memory* memory,
                                                   f32 value,
                                                   u32 n_samples,
                                                   u32 sample_duration);

internal DataGen* data_gen_create_random_spikes(Memory* memory,
                                                Random* random,
                                                f32 chance,
                                                u32 n_samples,
                                                u32 sample_duration);

internal DataGen* data_gen_create_spike_pulses(Memory* memory,
                                               Random* random,
                                               u32 n_samples,
                                               u32 sample_duration,
                                               u32 first_pulse_time,
                                               u32 pulse_duration,
                                               u32 between_pulses_duration,
                                               f32 pulse_spike_chance,
                                               f32 between_pulses_spike_chance);

internal DataGen* data_gen_create_spike_train(Memory* memory,
                                              u32 duration,
                                              const char* encodings_path,
                                              const char* listing_file,
                                              u32 max_time_to_use_from_train, // DEFAULT 0
                                              u32 n_samples);   // DEFAULT 0

internal DataGen* data_gen_create_basic_experiment(Memory* memory,
                                                   Random* random,
                                                   f32 spike_chance,
                                                   u32 n_samples);

internal DataGen* data_gen_create_background_activity(Memory* memory,
                                                      Random* random,
                                                      f32 chance_input,
                                                      f32 chance_background,
                                                      u32 n_samples,
                                                      u32 sample_duration);

typedef enum {
    DATA_SAMPLE_INVALID,
    DATA_SAMPLE_RANDOM_SPIKES,
    DATA_SAMPLE_CONSTANT_CURRENT,
    DATA_SAMPLE_SPIKE_PULSES,
    DATA_SAMPLE_SPIKE_TRAIN,
    DATA_SAMPLE_BASIC_EXPERIMENT,
    DATA_SAMPLE_BACKGROUND_ACTIVITY,
} DataSampleType;


typedef struct DataSampleSpikePulses {
    b32 in_pulse;
    u32 next_pulse_time;
    u32 next_between_pulses_time;
} DataSampleSpikePulses;

typedef struct DataSampleSpikeTrain {
    SpikeTrain* spikes;
} DataSampleSpikeTrain;

typedef struct DataSampleBasicExperiment {
    f32 spike_chance;
} DataSampleBasicExperiment;

typedef struct DataSampleBackgroundActivity
{
    f32 spike_chance_input;
    f32 spike_chance_background;
} DataSampleBackgroundActivity;

typedef struct DataSample {
    DataSampleType type;
    u32 duration;
    u32 sample_i;
    String* name;
    DataGen* data_gen;

    union {
        void* random_spikes;
        void* const_current;
        DataSampleSpikePulses spike_pulses;
        DataSampleSpikeTrain spike_train;
        DataSampleBasicExperiment basic_exp;
        DataSampleBackgroundActivity background_activity;
    };
} DataSample;

internal DataSample* data_gen_sample_create(Memory* memory, DataGen* data, u32 idx);

// NOTE: Currently the easiest thing to do is to give the network when creating inputs
// NOTE: Probably in the future this is not enough but we will see
internal Inputs* data_network_inputs_create(Memory* memory,
                                            DataSample* sample,
                                            Network* network,
                                            u32 time);


#endif // __ENGINE_DATA_GEN_H__
