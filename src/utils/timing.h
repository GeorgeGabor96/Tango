/* date = November 3rd 2022 8:27 pm */

#ifndef TIMING_H
#define TIMING_H

#include "common.h"

#define TIMING

// TODO: add a compile flag so that this module is not compiled
// TODO: add cl compile flag for the _rdtsc

#ifdef TIMING
typedef struct TimeCounter {
    u64 cycle_count;
    u32 hit_count;
} TimingCounter;


typedef enum {
    TIMER_SIMULATOR_RUN,
    TIMER_NETWORK_STEP,
    TIMER_LAYER_STEP,
    TIMER_LAYER_STEP_INJECT_CURRENT,
    TIMER_LAYER_STEP_FORCE_SPIKE,
    TIMER_NEURON_STEP,
    TIMER_NEURON_STEP_FORCE_SPIKE,
    TIMER_NEURON_STEP_INJECT_CURRENT,
    TIMER_SYNAPSE_STEP,
    TIMER_INVALID
} TimingCounterType;


TimingCounter timing_counters[128];

#define TIMING_COUNTER_START(NAME) u64 timing_start = __rdtsc();

#define TIMING_COUNTER_END(NAME) timing_counters[TIMER_##NAME].cycle_count += __rdtsc() - timing_start; \
++(timing_counters[TIMER_##NAME].hit_count);

internal const char* timing_counter_name(TimingCounterType type);
internal void timing_report();

#else

#define TIMING_COUNTER_START(NAME)
#define TIMING_COUNTER_END(NAME)

//#define timing_counter_name();
//#define timing_report();

#endif

#endif //TIMING_H
