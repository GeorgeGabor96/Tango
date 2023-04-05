internal InputSpikeMatrix*
input_spike_matrix_create(Memory* memory, u32 time_max, u32 n_neurons) {
    check(memory != NULL, "memory is NULL");
    check(time_max != 0, "time_max is 0");
    check(n_neurons != 0, "n_neurons is 0");

    InputSpikeMatrix* matrix = (InputSpikeMatrix*)memory_push_zero(memory, sizeof(InputSpikeMatrix) + sizeof(b32) * time_max * n_neurons);
    check_memory(matrix);
    matrix->time = time_max;
    matrix->n_neurons = n_neurons;
    matrix->data = (b32*)(matrix + 1);
    return matrix;

    error:
    return NULL;
}


internal void
input_spike_matrix_set(InputSpikeMatrix* matrix, u32 time, u32 neuron_i) {
    check(matrix != NULL, "matrix is NULL");
    check(time < matrix->time, "time >= matrix->time");
    check(neuron_i < matrix->n_neurons, "neuron_i >= matrix->n_neurons");

    matrix->data[time * matrix->n_neurons + neuron_i] = TRUE;

    error:
    return;
}


internal b32*
input_spike_matrix_get_spike_data_for_time(InputSpikeMatrix* matrix, u32 time) {
    check(matrix != NULL, "matrix is NULL");
    check(time < matrix->time, "time >= matrix->time");

    b32* result = matrix->data + time * matrix->n_neurons;
    return result;

    error:
    return NULL;
}


internal InputSpikeMatrix*
input_spike_matrix_create_from_spike_times(Memory* memory, InputSpikeTimes* spike_times) {
    check(memory != NULL, "memory is NULL");
    check(spike_times != NULL, "spike_times is NULL");
    check(spike_times->type == INPUT_SPIKE_TIMES_IMAGE,
        "spike_times->type is %s not %s",
        input_spike_times_get_type_c_str(spike_times->type),
        input_spike_times_get_type_c_str(INPUT_SPIKE_TIMES_IMAGE));

    InputSpikeMatrix* matrix = input_spike_matrix_create(memory, 256,
                    spike_times->image_info.width * spike_times->image_info.height);
    check_memory(matrix);

    for (u32 i = 0; i < spike_times->n_spike_times; ++i) {
        InputSpikeTimesPair* pair = spike_times->spike_times + i;
        input_spike_matrix_set(matrix, pair->spike_time, pair->neuron_i);
    }

    return matrix;

    error:
    return NULL;
}