#ifndef __SPIKE_TIMES_H__
#define __SPIKE_TIMES_H__


typedef enum  {
    INPUT_SPIKE_TIMES_IMAGE = 0,

    INPUT_SPIKE_TIMES_INVALID
} InputSpikeTimesType;


typedef struct InputSpikeTimesImage {
    u32 width;
    u32 height;
} InputSpikeTimesImage;

typedef struct InputSpikeTimesPair {
    u32 neuron_i;
    u32 spike_time;
} InputSpikeTimesPair;

typedef struct InputSpikeTimes {
    InputSpikeTimesType type;
    union {
        InputSpikeTimesImage image_info;
    };
    u32 n_spike_times;
    InputSpikeTimesPair* spike_times;
} InputSpikeTimes;

internal const char* input_spike_times_get_type_c_str(InputSpikeTimesType type);
internal InputSpikeTimes* input_spike_times_read(Memory* memory, const char* input_file);


#endif // __SPIKE_TIMES_H__