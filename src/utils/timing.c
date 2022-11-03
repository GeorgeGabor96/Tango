TimeCounter timing_counters[128];


internal const char*
timing_counter_name(TimingCounterType type) {
    if (type == TIMER_SIMULATOR_RUN) return "SIMULATOR_RUN";
    return "TIMER_INVALID";
}


internal void
timing_report() {
    u32 i = 0;
    TimingCounter* timer = NULL;
    
    printf("---------------TIMING REPORT----------------\n");
    for (i = 0; i < TIMER_INVALID; ++i) {
        timer = timing_counter + i;
        printf("%s\nCYCLE COUNT: %llu\nHIT COUNT: %u\n\n",
               timing_counter_name(i), timer->cycle_count, timer->hit_count);
    }
    
    // TODO: You can do like a total cycle count in all counters and compute
    // TODO: the percentage of each timer
}


