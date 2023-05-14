/*******************
* Helpfull functions
*******************/
internal b32
_synapse_is_spike_arriving(Synapse* synapse) {
    b32 result = synapse->spike_queue & 1;
    return result;
}


/***********************
* Synapse Learning Rule
***********************/
internal const char*
synapse_learning_rule_get_c_str(SynapseLearningRule rule) {
    if (rule == SYNAPSE_LEARNING_NO_LEARNING)
        return "SYNAPSE_LEARNING_NO_LEARNING";
    if (rule == SYNAPSE_LEARNING_EXPONENTIAL)
        return "SYNAPSE_LEARNING_EXPONENTIAL";
    if (rule == SYNAPSE_LEARNING_STEP)
        return "SYNAPSE_LEARNING_STEP";
    return "SYNAPSE_LEARNING_INVALID";
}

internal void
synapse_cls_add_learning_rule_exponential(SynapseCls* cls,
                                     f32 min_w, f32 max_w,
                                     f32 A, f32 B, f32 tau) {
    check(cls != NULL, "cls is NULL");
    check(min_w >= 0, "synapse weights should be positive. min_weight %f", min_w);
    check(max_w >= 0, "synapse weights should be positive. max_weight %f", max_w);
    check(min_w <= max_w, "min_w %f > max_w %f", min_w, max_w);
    cls->learning_rule = SYNAPSE_LEARNING_EXPONENTIAL;
    cls->min_w = min_w;
    cls->max_w = max_w;
    cls->stdp_exponential.A = A;
    cls->stdp_exponential.B = B;
    cls->stdp_exponential.tau = tau;

    error:
    return;
}

internal void
synapse_cls_add_learning_rule_step(SynapseCls* cls,
                                   f32 min_w, f32 max_w,
                                   u32 max_time_p, f32 amp_p,
                                   u32 max_time_d, f32 amp_d) {
    check(cls != NULL, "cls is NULL");
    check(min_w >= 0, "synapse weights should be positive. min_weight %f", min_w);
    check(max_w >= 0, "synapse weights should be positive. max_weight %f", max_w);
    check(min_w <= max_w, "min_w %f > max_w %f", min_w, max_w);
    cls->learning_rule = SYNAPSE_LEARNING_STEP;
    cls->min_w = min_w;
    cls->max_w = max_w;
    cls->stdp_step.max_time_p = max_time_p;
    cls->stdp_step.amp_p = amp_p;
    cls->stdp_step.max_time_d = max_time_d;
    cls->stdp_step.amp_d = amp_d;

    error:
    return;
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
                   String* name, SynapseType type,
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

    cls->name = name;
    cls->type = type;
    cls->rev_potential = rev_potential;
    cls->amp = amp;
    cls->tau_exp = math_exp_f32(-1/tau_ms);
    cls->delay = delay;

    // NOTE: init learning rule stuff
    cls->min_w = 0;
    cls->max_w = 0;
    cls->learning_rule = SYNAPSE_LEARNING_NO_LEARNING;

    return cls;

    error:
    return NULL;
}

internal SynapseCls*
synapse_cls_create_exci(Memory* memory, String* name, SynapseType type) {
    return synapse_cls_create(memory, name, type, 0.0f, 0.1f, 1, 10);
}

internal SynapseCls*
synapse_cls_create_inhi(Memory* memory, String* name, SynapseType type) {
    return synapse_cls_create(memory, name, type, -90.0f, 0.05, 5, 1);
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
    // NOTE: the weight should be positive because the inhibition or excitation is driven by the reversal potential
    // NOTE: not the weight
    check(weight >= 0.0f, "weight is negative");

    synapse->cls = cls;
    synapse->weight = weight;
    synapse->conductance = 0.0f;
    synapse->spike = FALSE;
    synapse->last_spike_time = INVALID_SPIKE_TIME;

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
        synapse->spike = TRUE;
        synapse->last_spike_time = time;
    } else {
        // NOTE: conductance should always be positive
        // NOTE: clip the conductance if its too low
        if (synapse->conductance > 0.001f) {
            synapse->conductance *= synapse->cls->tau_exp;
        } else {
            synapse->conductance = 0.0f;
        }
        synapse->spike = FALSE;
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
    synapse->spike = FALSE;
    synapse->last_spike_time = INVALID_SPIKE_TIME;
    synapse->spike_queue = 0;

    error:
    return;
}


internal void
synapse_learning_step(Synapse* synapse, u32 time) {
    TIMING_COUNTER_START(SYNAPSE_LEARNING_STEP);

    check(synapse != NULL, "synapse is NULL");
    synapse_step(synapse, time);

    if (synapse->spike == TRUE) {
        // NOTE: The spike arrived at the synapse, if the neuron spike before this,
        // need to reduce strength of this synapse
        synapse_depression(synapse, synapse->out_neuron->last_spike_time);
    }

    TIMING_COUNTER_END(SYNAPSE_LEARNING_STEP);

    error:
    return;
}


internal void
synapse_potentiation(Synapse* synapse, u32 neuron_spike_time) {
    // NOTE: it considers the time difference between the moment the synapse prosessed the spike
    // (the pre-spike travelled the axon and reached the synapse) and the time the post-neuron spiked.
    // The axonal dealy doesn't matter
    check(synapse != NULL, "synapse is NULL");

    SynapseCls* cls = synapse->cls;
    u32 synapse_spike_time = synapse->last_spike_time;

    // NOTE: The post_neuron spiked, the synapse should have also spiked to be able to do something
    if (synapse_spike_time == INVALID_SPIKE_TIME) return;
    // NOTE: if the neuron spike and the synapse spiked at the same moment, the effect of the synapse appears at the next step
    if (neuron_spike_time == synapse_spike_time) return;
    check(neuron_spike_time > synapse_spike_time,
          "post neuron time %u should be at least the synapse time %u",
          neuron_spike_time, synapse_spike_time);

    u32 dt = neuron_spike_time - synapse_spike_time;
    f32 dw = 0.0f;

    if (cls->learning_rule == SYNAPSE_LEARNING_NO_LEARNING) {
        dw = 0;
    } else if (cls->learning_rule == SYNAPSE_LEARNING_EXPONENTIAL) {
        SynapseLearningExponential* rule = &cls->stdp_exponential;
        dw = rule->A * math_exp_f32(-(f32)dt / rule->tau);
    } else if (cls->learning_rule == SYNAPSE_LEARNING_STEP) {
        SynapseLearningStep* rule = &cls->stdp_step;
        if (dt <= rule->max_time_p) dw = rule->amp_p;
    } else {
        log_error("Unkown Synapse Learning Rule %s (%u)",
        synapse_learning_rule_get_c_str(cls->learning_rule),
        cls->learning_rule);
    }

    synapse->weight = math_clip_f32(synapse->weight + dw, cls->min_w, cls->max_w);
    error:
    return;
}


internal void
synapse_depression(Synapse* synapse, u32 neuron_spike_time) {
    // NOTE: it considers the time difference between the moment the synapse prosessed the spike
    // (the pre-spike travelled the axon and reached the synapse) and the time the post-neuron spiked.
    // The axonal dealy doesn't matter
    check(synapse != NULL, "synapse is NULL");

    SynapseCls* cls = synapse->cls;
    u32 synapse_spike_time = synapse->last_spike_time;

    // NOTE: the synapse spiked, the post neuron should have also spiked to be able to do something
    if (neuron_spike_time == INVALID_SPIKE_TIME) return;
    // NOTE: if the neuron spike and the synapse spiked at the same moment, the effect of the synapse appears at the next step
    if (neuron_spike_time == synapse_spike_time) return;
    check(synapse_spike_time > neuron_spike_time,
          "synapse_spike_time %u should be at most the neuron_spike-time %u",
          synapse_spike_time, neuron_spike_time);

    u32 dt = synapse_spike_time - neuron_spike_time;
    u32 dw = 0.0f;

    if (cls->learning_rule == SYNAPSE_LEARNING_NO_LEARNING) {
        dw = 0;
    } else if (cls->learning_rule == SYNAPSE_LEARNING_EXPONENTIAL) {
        SynapseLearningExponential* rule = &cls->stdp_exponential;
        dw = rule->B * math_exp_f32(-(f32)dt / rule->tau);
    } else if (cls->learning_rule == SYNAPSE_LEARNING_STEP) {
        SynapseLearningStep* rule = &cls->stdp_step;
        if (dt <= rule->max_time_d) dw = rule->amp_d;
    } else {
        log_error("Unkown Synapse Learning Rule %s (%u)",
        synapse_learning_rule_get_c_str(cls->learning_rule),
        cls->learning_rule);
    }

    synapse->weight = math_clip_f32(synapse->weight + dw, cls->min_w, cls->max_w);

    error:
    return;
}
