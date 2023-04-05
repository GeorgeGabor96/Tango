#ifndef __INPUT_SPIKE_MATRIX_H__
#define __INPUT_SPIKE_MATRIX_H__


typedef struct InputSpikeMatrix {
    u32 time;
    u32 n_neurons;
    b32* data;
} InputSpikeMatrix;


// NOTE: always spikes are on false
internal InputSpikeMatrix* input_spike_matrix_create(Memory* memory, u32 time_max, u32 n_neurons);
internal void input_spike_matrix_set(InputSpikeMatrix* matrix, u32 time, u32 neuron_i);
internal b32* input_spike_matrix_get_spike_data_for_time(InputSpikeMatrix* matrix, u32 time);

internal InputSpikeMatrix* input_spike_matrix_create_from_spike_times(Memory* memory, InputSpikeTimes* spike_times);

#endif // __INPUT_SPIKE_MATRIX_H__