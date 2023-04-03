internal const char*
input_spike_times_get_type_c_str(InputSpikeTimesType type) {
    if (type == INPUT_SPIKE_TIMES_IMAGE) return "INPUT_SPIKE_TIMES_IMAGE";
    return "INPUT_SPIKE_TIMES_INVALID";
}


internal InputSpikeTimes*
input_spike_times_read(Memory* memory, const char* input_file) {
    check(memory != NULL, "memory is NULL");
    check(input_file != NULL, "input_file is NULL");

    FILE* fp = fopen(input_file, "rb");
    check(fp != NULL, "couldn't open file %s", input_file);

    InputSpikeTimes* input = (InputSpikeTimes*)memory_push(memory, sizeof(*input));
    check_memory(input);

    u32 n_neurons = 0;
    u32 type = 0;
    fread(&type, sizeof(u32), 1, fp);

    if (type == INPUT_SPIKE_TIMES_IMAGE) {
        fread(&input->image_info.height, sizeof(u32), 1, fp);
        fread(&input->image_info.width, sizeof(u32), 1, fp);
    } else {
        log_error("Invalid InputType %s", input_spike_times_get_type_c_str(type));
    }
    input->type = type;
    fread(&input->n_spike_times, sizeof(u32), 1, fp);

    input->spike_times = (InputSpikeTimesPair*)memory_push(memory, sizeof(InputSpikeTimesPair) * input->n_spike_times);
    check_memory(input->spike_times);
    fread(input->spike_times, sizeof(InputSpikeTimesPair), input->n_spike_times, fp);
    fclose(fp);

    return input;

    error:
    return NULL;
}