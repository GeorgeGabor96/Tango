/* date = November 3rd 2022 8:27 pm */

#ifndef TIMING_H
#define TIMING_H

#include "common.h"
#include "time.h"


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


// NOTE: need a version for each compiler
#ifdef _MSC_VER
#define timing_clock() __rdtsc()
#endif


#define TIMING_COUNTER_START(NAME) u64 timing_start = timing_clock();

#define TIMING_COUNTER_END(NAME) timing_counters[TIMER_##NAME].cycle_count += timing_clock() - timing_start; \
++(timing_counters[TIMER_##NAME].hit_count);

internal const char* timing_counter_name(TimingCounterType type);
internal void timing_report();

#else

#define TIMING_COUNTER_START(NAME)
#define TIMING_COUNTER_END(NAME)

#define timing_counter_name(type);
#define timing_report();

#endif

#endif //TIMING_H
