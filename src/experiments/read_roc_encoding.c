#include "tango.h"


int main() {
    Memory* memory = memory_create(MB(10), TRUE);
    InputSpikeTimes* encoding = input_spike_times_read(memory, "d:/datasets/MNIST/encoding/img_train/9999.bin");

    printf("Type %s\nHeight %u Width %u\n", input_spike_times_get_type_c_str(encoding->type), encoding->image_info.height, encoding->image_info.width);
    printf("N Spikes %u\n", encoding->n_spike_times);
    for (u32 i = 0; i < encoding->n_spike_times; ++i) {
        printf("%u %u\n", encoding->spike_times[i].neuron_i, encoding->spike_times[i].spike_time);
    }

    return 0;
}