#include "utils/timing.h"

#ifdef TIMING

internal const char*
timing_counter_name(TimingCounterType type) {
    if (type == TIMER_SIMULATOR_RUN) return "SIMULATOR_RUN";
    if (type == TIMER_NETWORK_STEP) return "NETWORK_STEP";
    if (type == TIMER_LAYER_STEP) return "LAYER_STEP";
    if (type == TIMER_LAYER_STEP_INJECT_CURRENT) return "LAYER_STEP_INJECT_CURRENT";
    if (type == TIMER_LAYER_STEP_FORCE_SPIKE) return "LAYER_STEP_FORCE_SPIKE";
    if (type == TIMER_NEURON_STEP) return "NEURON_STEP";
    if (type == TIMER_NEURON_STEP_FORCE_SPIKE) return "NEURON_STEP_FORCE_SPIKE";
    if (type == TIMER_NEURON_STEP_INJECT_CURRENT) return "NEURON_STEP_INJECT_CURRENT";
    if (type == TIMER_SYNAPSE_STEP) return "SYNAPSE_STEP";
    return "TIMER_INVALID";
}


internal void
timing_report() {
    u32 i = 0;
    TimingCounter* timer = NULL;
    
    printf("---------------TIMING REPORT----------------\n");
    for (i = 0; i < TIMER_INVALID; ++i) {
        timer = timing_counters + i;
        printf("%s:\n", timing_counter_name(i));
        printf("CYCLE COUNT: %llu\n", timer->cycle_count);
        printf("HIT COUNT: %u\n", timer->hit_count);
        if (timer->hit_count > 0)
            printf("CYCLES PER HIT: %.2lf\n\n", (f64)timer->cycle_count / timer->hit_count);
        else
            printf("\n");
    }
}
1
#endif // TIMING