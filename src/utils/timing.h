#ifndef __UTILS_TIMING_H__
#define __UTILS_TIMING_H__


#ifdef TIMING
typedef struct TimeCounter {
    u64 cycle_count;
    u32 hit_count;
} TimingCounter;


typedef enum {
    TIMER_EXPERIMENT_INFER,
    TIMER_EXPERIMENT_LEARN,

    TIMER_NETWORK_INFER,
    TIMER_NETWORK_LEARN,

    TIMER_LAYER_STEP,
    TIMER_LAYER_STEP_INJECT_CURRENT,
    TIMER_LAYER_STEP_FORCE_SPIKE,

    TIMER_NEURON_STEP,
    TIMER_NEURON_STEP_FORCE_SPIKE,
    TIMER_NEURON_STEP_INJECT_CURRENT,

    TIMER_SYNAPSE_STEP,
    TIMER_SYNAPSE_LEARNING_STEP,

    // LEARNING
    TIMER_NETWORK_LEARNING_STEP,

    TIMER_LAYER_LEARNING_STEP,
    TIMER_LAYER_LEARNING_STEP_INJECT_CURRENT,
    TIMER_LAYER_LEARNING_STEP_FORCE_SPIKE,

    TIMER_NEURON_LEARNING_STEP,
    TIMER_NEURON_LEARNING_STEP_FORCE_SPIKE,
    TIMER_NEURON_LEARNING_STEP_INJECT_CURRENT,

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
internal void timing_report(Memory* memory,
                            const char* output_folder_c_str);

#else

#define TIMING_COUNTER_START(NAME)
#define TIMING_COUNTER_END(NAME)

#define timing_counter_name(type) "TIMING NOTE COMPILED"
#define timing_report(memory, out_folder)

#endif

#endif // __UTILS_TIMING_H__
