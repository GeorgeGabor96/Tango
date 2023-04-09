#ifndef __SPIKE_TRAIN_H__
#define __SPIKE_TRAIN_H__


typedef enum {
    SPIKE_TRAIN_ENCODING_IMAGE = 0,
    SPIKE_TRAIN_OUTPUT = 1, // This will be file in another ticket

    SPIKE_TRAIN_INVALID
} SpikeTrainType;

internal const char* spike_train_type_get_c_str(SpikeTrainType type);


typedef struct SpikeTrainMetadataEncodingImage {
    u32 width;
    u32 height;
} SpikeTrainMetadataEncodingImage;


typedef struct SpikeTrain {
    SpikeTrainType type;
    union {
        SpikeTrainMetadataEncodingImage encoding_image;
    };
    u32 time_max;
    u32 n_neurons;
    b32* data; // dimension is (TIME, NEURONS) for easy access to all data for a time
} SpikeTrain;


internal SpikeTrain* spike_train_read(Memory* memory, String* path);
internal b32* spike_train_get_for_time(SpikeTrain* spikes, u32 time);

#endif // __INPUT_SPIKE_MATRIX_H__