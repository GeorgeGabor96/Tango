#include "utils/timing.h"


#ifdef TIMING

internal const char*
timing_counter_name(TimingCounterType type) {
    switch (type) {
        case TIMER_SIMULATOR_INFER:
            return "SIMULATOR_INFER";
        case TIMER_SIMULATOR_LEARN:
            return "SIMULATOR_LEARN";

        case TIMER_NETWORK_STEP:
            return "NETWORK_STEP";
    
        case TIMER_LAYER_STEP:
            return "LAYER_STEP";
        case TIMER_LAYER_STEP_INJECT_CURRENT:
            return "LAYER_STEP_INJECT_CURRENT";
        case TIMER_LAYER_STEP_FORCE_SPIKE:
            return "LAYER_STEP_FORCE_SPIKE";
    
        case TIMER_NEURON_STEP:
            return "NEURON_STEP";
        case TIMER_NEURON_STEP_FORCE_SPIKE:
            return "NEURON_STEP_FORCE_SPIKE";
        case TIMER_NEURON_STEP_INJECT_CURRENT:
            return "NEURON_STEP_INJECT_CURRENT";
    
        case TIMER_SYNAPSE_STEP:
            return "SYNAPSE_STEP";
    
        case TIMER_NETWORK_LEARNING_STEP:
            return "NETWORK_LEARNING_STEP";

        case TIMER_LAYER_LEARNING_STEP:
            return "LAYER_LEARNING_STEP";
        case TIMER_LAYER_LEARNING_STEP_INJECT_CURRENT:
            return "LAYER_LEARNING_STEP_INJECT_CURRENT";
        case TIMER_LAYER_LEARNING_STEP_FORCE_SPIKE:
            return "LAYER_LEARNING_STEP_FORCE_SPIKE";
        
        case TIMER_NEURON_LEARNING_STEP:
            return "NEURON_LEARNING_STEP";
        case TIMER_NEURON_LEARNING_STEP_INJECT_CURRENT:
            return "NEURON_LEARNING_STEP_INJECT_CURRENT";
        case TIMER_NEURON_LEARNING_STEP_FORCE_SPIKE:
            return "NEURON_LEARNING_STEP_FORCE_SPIKE";

        default:
            return "TIMER_INVALID";

    return "TIMER_INVALID";
    }
}


internal void
timing_report(MemoryArena* arena, const char* output_folder_c_str) {
    u32 i = 0;
    TimingCounter* timer = NULL;
    String* out_folder = NULL;
    String* out_file = NULL;
    FILE* fp = NULL;
    
    out_folder = string_create(arena, output_folder_c_str);
    
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
        out_file = string_path_join_c_str(arena, out_folder, buffer);
    }
    if (out_file == NULL) fp = stdout;
    else {
        fp = fopen(string_to_c_str(out_file), "w");
        if (fp == NULL) {
            printf("[WARNING] Could not open %s\n", string_to_c_str(out_file));
            fp = stdout;
        } else {
            printf("[INFO] Saving timings in %s\n", string_to_c_str(out_file));
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
