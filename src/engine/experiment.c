internal Experiment*
experiment_create(State* state, Network* network, DataGen* data) {
    Experiment* exp = NULL;

    check(state != NULL, "state is NULL");
    check(network != NULL, "network is NULL");
    check(data != NULL, "data is NULL");

    exp = (Experiment*)memory_push(state->permanent_storage, sizeof(*exp));
    check_memory(exp);

    exp->network = network;
    exp->data = data;
    exp->n_callbacks = 0;

    return exp;

    error:
    return NULL;
}


internal void
_experiment_run(Experiment* exp, State* state, ThreadPool* pool, Mode mode)
{
    check(exp != NULL, "exp is NULL");
    check(state != NULL, "state is NULL");
    check(pool != NULL, "pool is NULL");

    DataSample* sample = NULL;
    Inputs* inputs = NULL;
    u32 sample_idx = 0;
    u32 time = 0;
    u32 callback_i = 0;

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

        sample = data_gen_sample_create(state->transient_storage,
                                        exp->data, sample_idx);

        network_clear(exp->network);

        for (callback_i = 0; callback_i < exp->n_callbacks; ++callback_i)
            callback_begin_sample(state,
                                  exp->callbacks[callback_i],
                                  exp->network,
                                  sample->duration);

        for (time = 0; time < sample->duration; ++time) {
            inputs = data_network_inputs_create(state->transient_storage,
                                                sample,
                                                exp->network,
                                                time);

            network_time_start = clock();

            if (mode == MODE_INFER)
                network_infer(exp->network, inputs, time, state->transient_storage, pool);
            else if (mode == MODE_LEARNING)
                network_learn(exp->network, inputs, time, state->transient_storage, pool);
            else
                log_error("Unknown exp mode %u (%s)",
                          mode,
                          mode_get_c_str(mode));

            network_time += clock() - network_time_start;

            for (callback_i = 0;
                 callback_i < exp->n_callbacks;
                 ++callback_i)
                callback_update(state,
                                exp->callbacks[callback_i],
                                exp->network);
        }

        for (callback_i = 0; callback_i < exp->n_callbacks; ++callback_i)
            callback_end_sample(state,
                                exp->callbacks[callback_i],
                                exp->network);

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
experiment_infer(Experiment* sim, State* state, ThreadPool* pool) {
    TIMING_COUNTER_START(SIMULATOR_INFER);
    _experiment_run(sim, state, pool, MODE_INFER);
    TIMING_COUNTER_END(SIMULATOR_INFER);
}

internal void
experiment_learn(Experiment* sim, State* state, ThreadPool* pool) {
    TIMING_COUNTER_START(SIMULATOR_LEARN);
    _experiment_run(sim, state, pool, MODE_LEARNING);
    TIMING_COUNTER_END(SIMULATOR_LEARN);
}

internal void
experiment_add_callback(Experiment* exp, State* state, Callback* callback) {
    check(exp != NULL, "exp is NULL");
    check(state != NULL, "state is NULL");
    check(callback != NULL, "callback is NULL");
    check(exp->n_callbacks <= SIMULATOR_N_MAX_CALLBACKS,
          "Cannot add more callbacks: exp->n_callbacks %u, SIMULATOR_N_MAX_CALLBACKS %u",
          exp->n_callbacks, SIMULATOR_N_MAX_CALLBACKS);

    exp->callbacks[exp->n_callbacks] = callback;
    ++(exp->n_callbacks);

    error:
    return;
}
