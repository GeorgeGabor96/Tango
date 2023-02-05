/*******************
* Helpfull functions
*******************/
internal b32
_synapse_is_spike_arriving(Synapse* synapse) {
    b32 result = synapse->spike_queue & 1;
    return result;
}


/*******************
* Synapse Type
*******************/
internal char*
synapse_type_get_c_str(SynapseType type) {
    if (type == SYNAPSE_CONDUCTANCE) return "SYNAPSE_CONDUCTANCE";
    else if (type == SYNAPSE_VOLTAGE) return "SYNAPSE_VOLTAGE";
    else return "SYNAPSE_INVALID";
}


/*******************
* Synapse Class
*******************/
internal SynapseCls*
synapse_cls_create(Memory* memory,
                   const char* name, SynapseType type,
                   f32 rev_potential, f32 amp, f32 tau_ms, u32 delay) {
    check(memory != NULL, "Memory is NULL");
    check(name != NULL, "name is NULL");
    check(type == SYNAPSE_CONDUCTANCE || type == SYNAPSE_VOLTAGE,
          "invalid synapse type %s",
          synapse_type_get_c_str(type));
    check(rev_potential > -200.0f && rev_potential < 100.0f,
          "rev_potential is %f but it shouldn't be outside (-200, 100)",
          rev_potential);
    check(amp > 0.0f, "amp needs to be > 0");
    check(tau_ms > 0.0f, "tau_ms needs to be > 0");
    check(delay > 0 && delay <= SYNAPSE_QUEUE_CAPACITY,
          "delay needs to be in [0, %u] not %llu", delay, SYNAPSE_QUEUE_CAPACITY);

    SynapseCls* cls = (SynapseCls*)memory_push(memory, sizeof(*cls));
    check_memory(cls);

    cls->name = string_create(memory, name);
    check_memory(cls->name);

    cls->type = type;
    cls->rev_potential = rev_potential;
    cls->amp = amp;
    cls->tau_exp = (f32)math_op_exp((f64)(-1)/tau_ms);
    cls->delay = delay;
    return cls;

    error:
    return NULL;
}


/*******************
* Synapse
*******************/
internal Synapse*
synapse_create(Memory* memory, SynapseCls* cls, f32 weight) {
    check(memory != NULL, "memory is NULL");
    check(cls != NULL, "cls is NULL");

    Synapse* synapse = (Synapse*)memory_push(memory, sizeof(*synapse));
    check_memory(synapse);

    synapse_init(synapse, cls, weight);

    return synapse;
    error:
    return NULL;
}


internal void
synapse_init(Synapse* synapse, SynapseCls* cls, f32 weight) {
    check(synapse != NULL, "synapse is NULL");
    check(cls != NULL, "cls is NULL");
    check(weight >= 0.0f, "weight is negative");

    synapse->cls = cls;
    synapse->weight = weight;
    synapse->conductance = 0.0f;

    synapse->spike_queue = 0;

    synapse->in_neuron = NULL;
    synapse->out_neuron = NULL;

    error:
    return;
}


internal u32
synapse_next_spike_offset(Synapse* synapse) {
    check(synapse != NULL, "synapse is NULL");

    // NOTE: try to find the first set bit
    for (u32 i = 0; i < 64; ++i) {
        if (synapse->spike_queue & (1 << i)) return i;
    }

    error:
    return 0;
}


internal void
synapse_add_spike_time(Synapse* synapse, u32 spike_time) {
    check(synapse != NULL, "synapse is NULL");

    // NOTE: current spike offset should be larger than the one already in the queue
    check(synapse_next_spike_offset(synapse) < synapse->cls->delay,
           "Spike %u should not be older than the head %u",
           synapse->cls->delay,
           synapse_next_spike_offset(synapse));

    // NOTE: Just set the bit corresponding to the offset
    synapse->spike_queue |= 1 << synapse->cls->delay;

    error:
    return;
}


internal f32
synapse_compute_psc(Synapse* synapse, f32 voltage) {
    f32 current = 0.0f;
    check(synapse != NULL, "synapse is NULL");

    // TODO: check the formulas on paper
    SynapseCls* cls = synapse->cls;
    if (cls->type == SYNAPSE_CONDUCTANCE) {
        current = cls->amp * synapse->weight * synapse->conductance;
    } else if (cls->type == SYNAPSE_VOLTAGE) {
        current = - cls->amp * synapse->weight * synapse->conductance
            * (voltage - cls->rev_potential);
    } else {
        log_error("INVALID synapse type %u", cls->type);
    }

    error:
    return current;
}


internal void
synapse_step(Synapse* synapse, u32 time) {
    TIMING_COUNTER_START(SYNAPSE_STEP);

    check(synapse != NULL, "synapse is NULL");

    if (_synapse_is_spike_arriving(synapse)) {
        synapse->conductance += 1.0f;
    } else {
        // NOTE: conductance should always be positive
        // NOTE: clip the conductance if its too low
        if (synapse->conductance > 0.001f) {
            synapse->conductance *= synapse->cls->tau_exp;
        } else {
            synapse->conductance = 0.0f;
        }
    }

    // NOTE: Just shift the queue to the right
    synapse->spike_queue >>= 1;

    TIMING_COUNTER_END(SYNAPSE_STEP);

    error:
    return;
}


internal void
synapse_clear(Synapse* synapse) {
    check(synapse != NULL, "synapse is NULL");

    synapse->conductance = 0;
    synapse->spike_queue = 0;

    error:
    return;
}


internal f32
_synapse_stdp_potentiation_weight_update(u32 interval) {
    return 0.1f;
}

internal void
synapse_stdp_potentiation_update(Synapse* synapse, u32 in_spike_time, u32 out_spike_time) {
    check(synapse != NULL, "synapse is NULL");

    // NOTE: the input neuron should have spiked
    if (in_spike_time == NEURON_INVALID_SPIKE_TIME) return;

    u32 interval_value = out_spike_time - in_spike_time;
    // NOTE: the interval between the spike should be big enough and it should not be 0
    if (interval_value > SYNAPSE_POTENTIATION_INTERVAL ||
        interval_value < synapse->cls->delay)
        return;

    synapse->weight += _synapse_stdp_potentiation_weight_update(interval_value);
    // TODO: add an STDP Structure that the synapse class points to in the future
    if (synapse->weight > SYNAPSE_WEIGHT_MAX) {
        synapse->weight = SYNAPSE_WEIGHT_MAX;
    }

    error:
    return;
}


internal f32
_synapse_stdp_depression_weight_update(u32 interval) {
    return -0.001f;
}

internal void
synapse_stdp_depression_update(Synapse* synapse, u32 in_spike_time, u32 out_spike_time) {
    check(synapse != NULL, "synapse is NULL");

    // NOTE: the out neuron should have spiked
    if (out_spike_time == NEURON_INVALID_SPIKE_TIME) return;

    u32 interval_value = in_spike_time - out_spike_time;
    // NOTE: the interval between the spike should be big enough and it should not be 0
    if (interval_value > SYNAPSE_DEPRESSION_INTERVAL ||
        interval_value == 0)
        return;

    synapse->weight += _synapse_stdp_depression_weight_update(interval_value);
    if (synapse->weight > SYNAPSE_WEIGHT_MAX) {
        synapse->weight = SYNAPSE_WEIGHT_MAX;
    }

    error:
    return;
}

