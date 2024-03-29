#ifdef TIMING

internal const char*
timing_counter_name(TimingCounterType type) {
    if (type == TIMER_EXPERIMENT_INFER) return "EXPERIMENT_INFER";
    if (type == TIMER_EXPERIMENT_LEARN) return "EXPERIMENT_LEARN";

    if (type == TIMER_NETWORK_INFER) return "NETWORK_INFER";
    if (type == TIMER_NETWORK_LEARN) return "NETWORK_LEARN";

    if (type == TIMER_LAYER_STEP) return "LAYER_STEP";
    if (type == TIMER_LAYER_STEP_INJECT_CURRENT) return "LAYER_STEP_INJECT_CURRENT";
    if (type == TIMER_LAYER_STEP_FORCE_SPIKE) return "LAYER_STEP_FORCE_SPIKE";

    if (type == TIMER_NEURON_STEP) return "NEURON_STEP";
    if (type == TIMER_NEURON_STEP_FORCE_SPIKE) return "NEURON_STEP_FORCE_SPIKE";
    if (type == TIMER_NEURON_STEP_INJECT_CURRENT) return "NEURON_STEP_INJECT_CURRENT";

    if (type == TIMER_SYNAPSE_STEP) return "SYNAPSE_STEP";

    if (type == TIMER_NETWORK_LEARNING_STEP) return "NETWORK_LEARNING_STEP";

    if (type == TIMER_LAYER_LEARNING_STEP) return "LAYER_LEARNING_STEP";
    if (type == TIMER_LAYER_LEARNING_STEP_INJECT_CURRENT)
        return "LAYER_LEARNING_STEP_INJECT_CURRENT";
    if (type == TIMER_LAYER_LEARNING_STEP_FORCE_SPIKE) return "LAYER_LEARNING_STEP_FORCE_SPIKE";

    if (type == TIMER_NEURON_LEARNING_STEP) return "NEURON_LEARNING_STEP";
    if (type == TIMER_NEURON_LEARNING_STEP_INJECT_CURRENT)
        return "NEURON_LEARNING_STEP_INJECT_CURRENT";
    if (type == TIMER_NEURON_LEARNING_STEP_FORCE_SPIKE) return "NEURON_LEARNING_STEP_FORCE_SPIKE";

    return "TIMER_INVALID";
}


internal void
timing_report(Memory* memory, const char* output_folder_c_str) {
    u32 i = 0;
    TimingCounter* timer = NULL;
    String* out_folder = NULL;
    String* out_file = NULL;
    FILE* fp = NULL;

    out_folder = string_create(memory, output_folder_c_str);

    if (out_folder != NULL) {
        char buffer[256] = { 0 };
        time_t raw_time;
        time(&raw_time);
        struct tm* time_info = localtime(&raw_time);
        sprintf(buffer, "timing_d_%d_%d_%d_t_%d_%d_%d.txt",
                time_info->tm_mday,
                time_info->tm_mon + 1,
                time_info->tm_year + 1900,
                time_info->tm_hour,
                time_info->tm_min,
                time_info->tm_sec);
        out_file = string_path_join_c_str(memory, out_folder, buffer);
    }
    if (out_file == NULL) fp = stdout;
    else {
        fp = fopen(string_get_c_str(out_file), "w");
        if (fp == NULL) {
            log_warning("Could not open %s\n", string_get_c_str(out_file));
            fp = stdout;
        } else {
            log_info("Saving timings in %s\n", string_get_c_str(out_file));
        }
    }
    fprintf(fp, "---------------TIMING REPORT----------------\n");
    for (i = 0; i < TIMER_INVALID; ++i) {
        timer = timing_counters + i;
        fprintf(fp, "%s:\n", timing_counter_name(i));
        fprintf(fp, "CYCLE COUNT: %llu\n", timer->cycle_count);
        fprintf(fp, "HIT COUNT: %u\n", timer->hit_count);
        if (timer->hit_count > 0)
            fprintf(fp, "CYCLES PER HIT: %.2lf\n\n",
                    (f64)timer->cycle_count / timer->hit_count);
        else
            fprintf(fp, "\n");
    }

    if (fp != stdout) {
        fflush(fp);
        fclose(fp);
    }

    return;
}

#endif // TIMING
