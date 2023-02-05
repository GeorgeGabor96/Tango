#ifndef __ENGINE_DATA_GEN_H__
#define __ENGINE_DATA_GEN_H__


typedef enum {
    DATA_GEN_INVALID,
    DATA_GEN_CONSTANT_CURRENT,
    DATA_GEN_RANDOM_SPIKES,
    DATA_GEN_SPIKE_PULSES,
} DataGenType;


typedef struct DataGenConstCurrent {
    f32 value;
} DataGenConstCurrent;

typedef struct DataGenRandomSpikes {
    f32 chance;
} DataGenRandomSpikes;

typedef struct DataGenSpikePulses {
    u32 first_pulse_time;
    u32 pulse_duration;
    u32 between_pulses_duration;
    f32 pulse_spike_chance;
    f32 between_pulses_spike_chance;
} DataGenSpikePulses;


typedef struct DataGen {
    DataGenType type;
    u32 n_samples;
    // NOTE: This may be specific per type but for now its good here
    u32 sample_duration;

    union {
        DataGenConstCurrent const_current;
        DataGenRandomSpikes random_spikes;
        DataGenSpikePulses spike_pulses;
    };
} DataGen;


internal DataGen* data_gen_create_constant_current(Memory* memory,
                                                   f32 value,
                                                   u32 n_samples,
                                                   u32 sample_duration);
internal DataGen* data_gen_create_random_spikes(Memory* memory,
                                                f32 chance,
                                                u32 n_samples,
                                                u32 sample_duration);
internal DataGen* data_gen_create_spike_pulses(Memory* memory,
                                               u32 n_samples,
                                               u32 sample_duration,
                                               u32 first_pulse_time,
                                               u32 pulse_duration,
                                               u32 between_pulses_duration,
                                               f32 pulse_spike_chance,
                                               f32 between_pulses_spike_chance);

typedef enum {
    DATA_SAMPLE_INVALID,
    DATA_SAMPLE_RANDOM_SPIKES,
    DATA_SAMPLE_CONSTANT_CURRENT,
    DATA_SAMPLE_SPIKE_PULSES,
} DataSampleType;


typedef struct DataSampleSpikePulses {
    b32 in_pulse;
    u32 next_pulse_time;
    u32 next_between_pulses_time;
} DataSampleSpikePulses;


typedef struct DataSample {
    DataSampleType type;
    u32 duration;
    DataGen* data_gen;

    union {
        void* random_spikes;
        void* const_current;
        DataSampleSpikePulses spike_pulses;
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
