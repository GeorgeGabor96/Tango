internal Experiment*
experiment_create(State* state, Network* network, DataGen* data) {
    Experiment* experiment = NULL;

    check(state != NULL, "state is NULL");
    check(network != NULL, "network is NULL");
    check(data != NULL, "data is NULL");

    experiment = (Experiment*)memory_push(state->permanent_storage, sizeof(*experiment));
    check_memory(experiment);

    experiment->network = network;
    experiment->data = data;
    experiment->callbacks = NULL;

    return experiment;

    error:
    return NULL;
}


internal void
_experiment_run(Experiment* experiment, State* state, ThreadPool* pool, Mode mode)
{
    check(experiment != NULL, "experiment is NULL");
    check(state != NULL, "state is NULL");
    check(pool != NULL, "pool is NULL");

    CallbackLink* callback_it = NULL;
    DataSample* sample = NULL;
    Inputs* inputs = NULL;
    u32 sample_idx = 0;
    u32 time = 0;

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
    for (sample_idx = 0; sample_idx < experiment->data->n_samples; ++sample_idx) {
        sample_time_start = clock();

        sample = data_gen_sample_create(state->transient_storage,
                                        experiment->data, sample_idx);

        network_clear(experiment->network);

        for (callback_it = experiment->callbacks;
             callback_it != NULL;
             callback_it = callback_it->next)
            callback_begin_sample(state,
                                  callback_it->callback,
                                  experiment->network,
                                  sample->duration);

        for (time = 0; time < sample->duration; ++time) {
            inputs = data_network_inputs_create(state->transient_storage,
                                                sample,
                                                experiment->network,
                                                time);

            network_time_start = clock();

            if (mode == MODE_INFER)
                network_infer(experiment->network, inputs, time, state->transient_storage, pool);
            else if (mode == MODE_LEARNING)
                network_learn(experiment->network, inputs, time, state->transient_storage, pool);
            else
                log_error("Unknown experiment mode %u (%s)",
                          mode,
                          mode_get_c_str(mode));

            network_time += clock() - network_time_start;

            for (callback_it = experiment->callbacks;
                callback_it != NULL;
                callback_it = callback_it->next)
                callback_update(state,
                                callback_it->callback,
                                experiment->network);
        }

        for (callback_it = experiment->callbacks;
             callback_it != NULL;
             callback_it = callback_it->next)
            callback_end_sample(state,
                                callback_it->callback,
                                experiment->network);

        memory_clear(state->transient_storage);

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
experiment_infer(Experiment* exp, State* state, ThreadPool* pool) {
    TIMING_COUNTER_START(SIMULATOR_INFER);
    _experiment_run(exp, state, pool, MODE_INFER);
    TIMING_COUNTER_END(SIMULATOR_INFER);
}

internal void
experiment_learn(Experiment* exp, State* state, ThreadPool* pool) {
    TIMING_COUNTER_START(SIMULATOR_LEARN);
    _experiment_run(exp, state, pool, MODE_LEARNING);
    TIMING_COUNTER_END(SIMULATOR_LEARN);
}

internal void
experiment_add_callback(Experiment* experiment, State* state, Callback* callback) {
    check(experiment != NULL, "experiment is NULL");
    check(state != NULL, "state is NULL");
    check(callback != NULL, "callback is NULL");

    CallbackLink* link = (CallbackLink*)memory_push(state->permanent_storage, sizeof(*link));
    check_memory(link);

    link->callback = callback;
    link->next = experiment->callbacks ? experiment->callbacks : NULL;
    experiment->callbacks = link;

    error:
    return;
}
