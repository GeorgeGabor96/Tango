/********************
*   NEURON TYPE
********************/
internal char*
neuron_type_get_c_str(NeuronType type) {
    if (type == NEURON_LIF) return "NEURON_LIF";
    else if (type == NEURON_LIF_REFRACT) return "NEURON_LIF_REFRACT";
    else return "NEURON_INVALID";
}


/********************
*   NEURON CLASS
********************/
internal NeuronCls*
neuron_cls_create_lif(State* state, const char* name) {
    NeuronCls* neuron_cls = NULL;

    check(state != NULL, "state is NULL");
    check(name != NULL, "name is NULL");
    neuron_cls = (NeuronCls*)memory_push(state->permanent_storage, sizeof(*neuron_cls));
    check_memory(neuron_cls);

    neuron_cls->name = string_create(state->permanent_storage, name);
    check_memory(neuron_cls->name);
    neuron_cls->type = NEURON_LIF_REFRACT;

    return neuron_cls;

    error:
    return NULL;
}


internal NeuronCls*
neuron_cls_create_lif_refract(State* state, const char* name, u32 refract_time) {
    NeuronCls* neuron_cls = NULL;

    check(state != NULL, "state is NULL");
    check(name != NULL, "name is NULL");
    neuron_cls = (NeuronCls*)memory_push(state->permanent_storage, sizeof(NeuronCls));
    check_memory(neuron_cls);

    neuron_cls->name = string_create(state->permanent_storage, name);
    check_memory(neuron_cls->name);

    neuron_cls->type = NEURON_LIF_REFRACT;
    neuron_cls->lif_refract_cls.refract_time = refract_time;

    return neuron_cls;

    error:
    return NULL;
}


/********************
*   NEURON
********************/
internal Neuron*
neuron_create(State* state, NeuronCls* cls) {
    Neuron* neuron = NULL;

    check(state != NULL, "state is NULL");
    check(cls != NULL, "cls is NULL");
    neuron = (Neuron*)memory_push(state->permanent_storage, sizeof(Neuron));
    check_memory(neuron);

    neuron_init(neuron, cls);

    return neuron;

    error:
    return NULL;
}


internal void
neuron_init(Neuron* neuron, NeuronCls* cls) {
    check(neuron != NULL, "neuron is NULL");
    check(cls != NULL, "cls is NULL");

    neuron->cls = cls;

    if (neuron->cls->type == NEURON_LIF) {
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
    } else if (neuron->cls->type == NEURON_LIF_REFRACT) {
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
    }

    neuron->epsc = 0.0f;
    neuron->ipsc = 0.0f;
    neuron->spike = FALSE;
    neuron->last_spike_time = NEURON_INVALID_SPIKE_TIME;

    neuron->in_synapse_arrays = NULL;
    neuron->out_synapse_arrays = NULL;
    neuron->n_in_synapse_arrays = 0;
    neuron->n_out_synapse_arrays = 0;

    error:
    return;
}


internal f32
_neuron_compute_psc(Neuron* neuron, Synapse* synapses, u32 time) {
    f32 epsc = 0.0f;
    f32 ipsc = 0.0f;
    f32 current = 0.0f;
    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    SynapseIdxArray* it = NULL;

    for (it = neuron->in_synapse_arrays; it != NULL; it = it->next) {
        for (synapse_i = 0; synapse_i < it->length; ++synapse_i) {
            synapse = synapses + it->idxs[synapse_i];
            synapse_step(synapse, time);

            current = synapse_compute_psc(synapse, neuron->voltage);

            if (current >= 0) epsc += current;
            else ipsc += current;
        }
    }

    neuron->epsc = epsc;
    neuron->ipsc = ipsc;
    return epsc + ipsc;
}


internal void
_neuron_lif_voltage_update(Neuron* neuron, f32 psc) {
    neuron->voltage = NEURON_LIF_VOLTAGE_FACTOR * neuron->voltage +
        NEURON_LIF_I_FACTOR * psc +
        NEURON_LIF_FREE_FACTOR;
}


internal void
_neuron_update(Neuron* neuron, u32 time, f32 psc) {
    bool spike = FALSE;

    NeuronCls* cls = neuron->cls;

    if (neuron->cls->type == NEURON_LIF) {
        _neuron_lif_voltage_update(neuron, psc);
        if (neuron->voltage >= NEURON_LIF_VOLTAGE_TH) {
            neuron->voltage = NEURON_LIF_VOLTAGE_REST;
            spike = TRUE;
            neuron->last_spike_time = time;
        }
    } else if (neuron->cls->type == NEURON_LIF_REFRACT) {
        // NOTE: Only update after refractory period
        if (time - neuron->last_spike_time <= cls->lif_refract_cls.refract_time) {
            neuron->voltage = NEURON_LIF_VOLTAGE_REST;
            spike = FALSE;
        } else {
            _neuron_lif_voltage_update(neuron, psc);
            if (neuron->voltage >= NEURON_LIF_VOLTAGE_TH) {
                neuron->voltage = NEURON_LIF_VOLTAGE_REST;
                spike = TRUE;
                neuron->last_spike_time = time;
            }
        }
    } else {
        log_error("INVALID neuron type %u", cls->type);
    }
    neuron->spike = spike;
}


internal void
_neuron_update_in_synapses(Neuron* neuron, Synapse* synapses, u32 time) {
    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    SynapseIdxArray* it = NULL;

    for (it = neuron->in_synapse_arrays; it != NULL; it = it->next) {
        for (synapse_i = 0; synapse_i < it->length; ++synapse_i) {
            synapse = synapses + it->idxs[synapse_i];
            synapse_step(synapse, time);
        }
    }
}


internal void
_neuron_update_out_synapses(Neuron* neuron, Synapse* synapses, u32 time) {
    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    SynapseIdxArray* it = NULL;

    for (it = neuron->in_synapse_arrays; it != NULL; it = it->next) {
        for (synapse_i = 0; synapse_i < it->length; ++synapse_i) {
            synapse = synapses + it->idxs[synapse_i];
            synapse_add_spike_time(synapse, time);
        }
    }
}


internal void
neuron_step(Neuron* neuron, Synapse* synapses, u32 time) {
    TIMING_COUNTER_START(NEURON_STEP);

    check(neuron != NULL, "neuron is NULL");

    f32 psc = _neuron_compute_psc(neuron, synapses, time);
    _neuron_update(neuron, time, psc);
    _neuron_update_out_synapses(neuron, synapses, time);

    TIMING_COUNTER_END(NEURON_STEP);

    error:
    return;
}


internal void
neuron_step_force_spike(Neuron* neuron, Synapse* synapses, u32 time) {
    TIMING_COUNTER_START(NEURON_STEP_FORCE_SPIKE);

    check(neuron != NULL, "neuron is NULL");

    _neuron_update_in_synapses(neuron, synapses, time);
    neuron->spike = TRUE;
    neuron->last_spike_time = time;

    if (neuron->cls->type == NEURON_LIF) {
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
    } else if (neuron->cls->type == NEURON_LIF_REFRACT) {
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
    } else {
        log_error("INVALID neuron type %u", neuron->cls->type);
    }
    _neuron_update_out_synapses(neuron, synapses, time);

    TIMING_COUNTER_END(NEURON_STEP_FORCE_SPIKE);

    error:
    return;
}


internal void
neuron_step_inject_current(Neuron* neuron, Synapse* synapses, f32 psc, u32 time) {
    TIMING_COUNTER_START(NEURON_STEP_INJECT_CURRENT);

    check(neuron != NULL, "neuron is NULL");

    psc = _neuron_compute_psc(neuron, synapses, time) + psc;
    _neuron_update(neuron, time, psc);
    _neuron_update_out_synapses(neuron, synapses, time);

    TIMING_COUNTER_END(NEURON_STEP_INJECT_CURRENT);

    error:
    return;
}


internal void
neuron_clear(Neuron* neuron, Synapse* synapses) {
    check(neuron != NULL, "neuron is NULL");
    check(synapses != NULL, "synapses is NULL");

    neuron->spike = FALSE;
    if (neuron->cls->type == NEURON_LIF)
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
    else if (neuron->cls->type == NEURON_LIF_REFRACT)
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
    else
        log_error("Unknown neuron type %u", neuron->cls->type);

    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    SynapseIdxArray* it = NULL;

    for (it = neuron->in_synapse_arrays; it != NULL; it = it->next) {
        for (synapse_i = 0; synapse_i < it->length; ++synapse_i) {
            synapse = synapses + it->idxs[synapse_i];
            synapse_clear(synapse);
        }
    }

    error:
    return;
}


/***************
*  LEARNING
***************/
internal void
_neuron_learning_update_synapses(Neuron* neuron, Neuron* neurons, Synapse* synapses, u32 time) {
    u32 in_spike_time = 0;
    u32 out_spike_time = 0;
    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    SynapseIdxArray* it = NULL;

    if (neuron->spike == FALSE) return;

    // Backpropagating signal to input synapses
    for (it = neuron->in_synapse_arrays; it != NULL; it = it->next) {
        for (synapse_i = 0; synapse_i < it->length; ++synapse_i) {
            synapse = synapses + it->idxs[synapse_i];

            u32 in_spike_time = (neurons + synapse->in_neuron_i)->last_spike_time;
            u32 out_spike_time = neuron->last_spike_time;
            synapse_stdp_potentiation_update(synapse, in_spike_time, out_spike_time);
        }
    }

    // Backpropagating signal to output synapses
    for (it = neuron->out_synapse_arrays; it != NULL; it = it->next) {
        for (synapse_i = 0; synapse_i < it->length; ++synapse_i) {
            synapse = synapses + it->idxs[synapse_i];

            u32 in_spike_time = neuron->last_spike_time;
            u32 out_spike_time = (neurons + synapse->out_neuron_i)->last_spike_time;

            synapse_stdp_depression_update(synapse, in_spike_time, out_spike_time);
        }
    }
}


internal void
neuron_learning_step(Neuron* neuron, Neuron* neurons, Synapse* synapses, u32 time) {
    TIMING_COUNTER_START(NEURON_LEARNING_STEP);

    check(neuron != NULL, "nueorn is NULL");
    check(neurons != NULL, "neurons is NULL");
    check(synapses != NULL, "synapses is NULL");
    neuron_step(neuron, synapses, time);
    _neuron_learning_update_synapses(neuron, neurons, synapses, time);

    TIMING_COUNTER_END(NEURON_LEARNING_STEP);

    error:
    return;
}


internal void
neuron_learning_step_force_spike(Neuron* neuron, Neuron* neurons, Synapse* synapses, u32 time) {
    TIMING_COUNTER_START(NEURON_LEARNING_STEP_FORCE_SPIKE);
    check(neuron != NULL, "neuron is NULL");

    neuron_step_force_spike(neuron, synapses, time);
    _neuron_learning_update_synapses(neuron, neurons, synapses, time);

    TIMING_COUNTER_END(NEURON_LEARNING_STEP_FORCE_SPIKE);

    error:
    return;
}


internal void
neuron_learning_step_inject_current(Neuron* neuron, Neuron* neurons, Synapse* synapses, f32 psc, u32 time) {
    TIMING_COUNTER_START(NEURON_LEARNING_STEP_INJECT_CURRENT);
    check(neuron != NULL, "neuron is NULL");

    neuron_step_inject_current(neuron, synapses, psc, time);
    _neuron_learning_update_synapses(neuron, neurons, synapses, time);

    TIMING_COUNTER_END(NEURON_LEARNING_STEP_INJECT_CURRENT);

    error:
    return;
}
