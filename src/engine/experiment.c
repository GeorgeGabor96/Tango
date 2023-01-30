internal Experiment*
experiment_create(u32 n_workers, const char* output_folder) {

    Memory* permanent_memory = memory_create(BLOCK_SIZE, TRUE);
    check_memory(permanent_memory);

    Memory* transient_memory = memory_create(BLOCK_SIZE, TRUE);
    check_memory(transient_memory);

    Experiment* exp = (Experiment*)memory_push(permanent_memory, sizeof(*exp));
    check_memory(exp);

    ThreadPool* pool = thread_pool_create(n_workers,
                                          layer_process_neurons,
                                          permanent_memory);
    check_memory(exp);

    String* out_folder = string_create(permanent_memory, output_folder);
    check_memory(out_folder);

    exp->permanent_memory = permanent_memory;
    exp->transient_memory = transient_memory;
    exp->pool = pool;
    exp->network = NULL;
    exp->data = NULL;
    exp->callback_list = NULL;
    exp->n_callbacks = 0;

    return exp;

    error:
    return NULL;
}

internal void
experiment_destroy(Experiment* exp) {
    check(exp != NULL, "exp is NULL");

    thread_pool_stop(exp->pool);

    // NOTE: Just destroy the memory, everything is in there
    memory_destroy(exp->transient_memory);
    memory_destroy(exp->permanent_memory);

    error:
    return;
}


internal void
_experiment_run(Experiment* exp, Mode mode)
{
    check(exp != NULL, "exp is NULL");

    DataSample* sample = NULL;
    Inputs* inputs = NULL;
    u32 sample_idx = 0;
    u32 time = 0;
    CallbackLink* callback_it = NULL;

    // TIMING
    clock_t total_time_start = 0;
    clock_t total_time = 0;

    clock_t sample_time_start = 0;
    clock_t sample_time = 0;
    f64 sample_time_s = 0.0;

    clock_t network_time_start = 0;
    clock_t network_time = 0;
    f64 network_time_s = 0.0;

    random_init();

    total_time_start = clock();
    for (sample_idx = 0; sample_idx < exp->data->n_samples; ++sample_idx) {
        sample_time_start = clock();

        sample = data_gen_sample_create(exp->transient_memory, exp->data, sample_idx);

        network_clear(exp->network);

        for (callback_it = exp->callback_list; callback_it != NULL; callback_it = callback_it->next)
            callback_begin_sample(exp, callback_it->callback, sample->duration);

        for (time = 0; time < sample->duration; ++time) {
            inputs = data_network_inputs_create(exp, sample, time);

            network_time_start = clock();

            if (mode == MODE_INFER)
                network_infer(exp, inputs, time);
            else if (mode == MODE_LEARNING)
                network_learn(exp, inputs, time);
            else
                log_error("Unknown exp mode %u (%s)", mode, mode_get_c_str(mode));

            network_time += clock() - network_time_start;

            for (callback_it = exp->callback_list; callback_it != NULL; callback_it = callback_it->next)
                callback_update(exp, callback_it->callback);
        }

        for (callback_it = exp->callback_list; callback_it != NULL; callback_it = callback_it->next)
            callback_end_sample(exp, callback_it->callback);

        memory_clear(exp->transient_memory);

        sample_time = clock() - sample_time_start;

        // NOTE: Timing logging
        network_time_s = (f64)network_time / CLOCKS_PER_SEC;
        printf("\n");
        log_info("Network time %lfs", network_time_s);
        log_info("Network step time %lfs",
                 network_time_s / (f64)sample->duration);
        network_time = 0;

        sample_time_s = (f64)sample_time / CLOCKS_PER_SEC;
        log_info("Sample time %lfs", sample_time_s);
        log_info("Non NETWORK time %lfs", sample_time_s - network_time_s);
        sample_time = 0;

        printf("\n");
    }
    total_time = clock() - total_time_start;
    log_info("Total simulation time %lfs\n", (f64)total_time / CLOCKS_PER_SEC);

    error:
    return;
}


internal void
experiment_infer(Experiment* sim) {
    TIMING_COUNTER_START(EXPERIMENT_INFER);
    _experiment_run(sim, MODE_INFER);
    TIMING_COUNTER_END(EXPERIMENT_INFER);
}

internal void
experiment_learn(Experiment* sim, State* state, ThreadPool* pool) {
    TIMING_COUNTER_START(EXPERIMENT_LEARN);
    _experiment_run(sim, MODE_LEARNING);
    TIMING_COUNTER_END(EXPERIMENT_LEARN);
}


internal void
experiment_timing_report(Experiment* exp) {
    check(exp != NULL, "exp is NULL");

    timing_report(exp->transient_memory, exp->output_folder);
    error:

    return;
}


internal void
experiment_memory_report(Experiment* exp) {
    check(exp != NULL, "exp is NULL");

    memory_report();
    check(memory_is_empty() == TRUE, "We have memory leaks");

    error:
    return;
}