internal Experiment*
experiment_create(u32 n_workers, u32 seed) {
    Memory* permanent_memory = NULL;
    Memory* transient_memory = NULL;

    permanent_memory = (Memory*)memory_create(MB(1), TRUE);
    check_memory(permanent_memory);

    transient_memory = (Memory*)memory_create(MB(1), TRUE);
    check_memory(transient_memory);

    ThreadPool* pool = thread_pool_create(n_workers, layer_process_neurons, permanent_memory);
    check_memory(pool);

    Random* random = random_create(permanent_memory, seed);
    check_memory(random);

    Experiment* experiment = (Experiment*)memory_push(permanent_memory, sizeof(*experiment));
    check_memory(experiment);

    experiment->permanent_memory = permanent_memory;
    experiment->transient_memory = transient_memory;
    experiment->pool = pool;
    experiment->random = random;

    experiment->network = NULL;
    experiment->data = NULL;
    experiment->callbacks = NULL;

    return experiment;

    error:
    if (permanent_memory) memory_destroy(permanent_memory);
    if (transient_memory) memory_destroy(transient_memory);

    return NULL;
}


internal void
experiment_destroy(Experiment* experiment) {
    check(experiment != NULL, "experiment is NULL");

    thread_pool_stop(experiment->pool);
    memory_destroy(experiment->transient_memory);
    memory_destroy(experiment->permanent_memory);

    error:
    return;
}


internal void
_experiment_run(Experiment* experiment, Mode mode) {
    check(experiment != NULL, "experiment is NULL");
    check(experiment->data != NULL, "data is not set");
    check(experiment->network != NULL, "network is not set");

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

    total_time_start = clock();
    for (sample_idx = 0; sample_idx < experiment->data->n_samples; ++sample_idx) {
        sample_time_start = clock();

        sample = data_gen_sample_create(experiment->transient_memory,
                                        experiment->data, sample_idx);

        network_clear(experiment->network);

        for (callback_it = experiment->callbacks;
             callback_it != NULL;
             callback_it = callback_it->next)
            callback_begin_sample(callback_it->callback,
                                  sample->duration,
                                  experiment->transient_memory);

        for (time = 0; time < sample->duration; ++time) {
            inputs = data_network_inputs_create(experiment->transient_memory,
                                                sample,
                                                experiment->network,
                                                time);

            network_time_start = clock();

            if (mode == MODE_INFER)
                network_infer(experiment->network, inputs, time, experiment->transient_memory, experiment->pool);
            else if (mode == MODE_LEARNING)
                network_learn(experiment->network, inputs, time, experiment->transient_memory, experiment->pool);
            else
                log_error("Unknown experiment mode %u (%s)",
                          mode,
                          mode_get_c_str(mode));

            network_time += clock() - network_time_start;

            for (callback_it = experiment->callbacks;
                callback_it != NULL;
                callback_it = callback_it->next)
                callback_update(callback_it->callback, experiment->transient_memory);
        }

        for (callback_it = experiment->callbacks;
             callback_it != NULL;
             callback_it = callback_it->next)
            callback_end_sample(callback_it->callback, experiment->transient_memory);

        memory_clear(experiment->transient_memory);

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
experiment_infer(Experiment* exp) {
    TIMING_COUNTER_START(SIMULATOR_INFER);
    _experiment_run(exp, MODE_INFER);
    TIMING_COUNTER_END(SIMULATOR_INFER);
}

internal void
experiment_learn(Experiment* exp) {
    TIMING_COUNTER_START(SIMULATOR_LEARN);
    _experiment_run(exp, MODE_LEARNING);
    TIMING_COUNTER_END(SIMULATOR_LEARN);
}

internal b32
experiment_add_callback(Experiment* experiment, Callback* callback) {
    check(experiment != NULL, "experiment is NULL");
    check(callback != NULL, "callback is NULL");

    CallbackLink* link = (CallbackLink*)memory_push(experiment->permanent_memory, sizeof(*link));
    check_memory(link);

    link->callback = callback;
    link->next = experiment->callbacks ? experiment->callbacks : NULL;
    experiment->callbacks = link;

    return TRUE;
    error:
    return FALSE;
}

internal b32
experiment_set_network(Experiment* experiment, Network* network) {
    check(experiment != NULL, "experiment is NULL");
    check(network != NULL, "network is NULL");

    check(experiment->network == NULL, "the network is already set");

    experiment->network = network;

    return TRUE;
    error:
    return FALSE;
}

internal b32
experiment_set_data_gen(Experiment* experiment, DataGen* data) {
    check(experiment != NULL, "experiment is NULL");
    check(data != NULL, "data is NULL");

    check(experiment->data == NULL, "the data gen is already set");

    experiment->data = data;

    return TRUE;
    error:
    return FALSE;
}