#include "simulator/synapse.h"


/*******************
* Helpfull functions
*******************/
inline internal bool
synapse_is_spike_arriving_at_this_time(Synapse* synapse, u32 time) {
    bool result = synapse->n_spike_times != 0 &&
        synapse->spike_times[synapse->spike_times_head] == time;
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
synapse_cls_create(State* state, 
                   const char* name, SynapseType type,
                   f32 rev_potential, f32 amp, f32 tau_ms, u32 delay) {
    SynapseCls* cls = NULL;
    
    check(state != NULL, "state is NULL");
    check(name != NULL, "name is NULL");
    check(type == SYNAPSE_CONDUCTANCE || type == SYNAPSE_VOLTAGE,
          "invalid synapse type %s",
          synapse_type_get_c_str(type));
    check(rev_potential > -200.0f && rev_potential < 100.0f, 
          "rev_potential is %f but it shouldn't be outside (-200, 100)",
          rev_potential);
    check(amp > 0.0f, "amp needs to be > 0");
    check(tau_ms > 0.0f, "tau_ms needs to be > 0");
    check(delay > 0, "delay needs to be > 0");
    
    cls = (SynapseCls*)memory_arena_push(state->permanent_storage, sizeof(*cls));
    check_memory(cls);
    
    cls->name = string_create(state->permanent_storage, name);
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
internal sz
synapse_size_with_cls(SynapseCls* cls) {
    sz result = sizeof(Synapse) + (cls->delay + 1) * sizeof(u32);
    return result;
}


internal Synapse*
synapse_create(State* state, SynapseCls* cls, f32 weight) {
    Synapse* synapse = NULL;
    
    check(state != NULL, "state is NULL");
    check(cls != NULL, "cls is NULL");
    
    // Alloc also the queue after the synapse for max cache
    synapse = (Synapse*)memory_arena_push(state->permanent_storage,
                                          synapse_size_with_cls(cls));
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
    
    // NOTE: Depending on the order of update and add spike time
    // NOTE: you 
    // need a number of delay slots in the queue (in case update then add)
    // NOTE: or delay + 1 (in case add then update)
    // NOTE: Recheck this, for now its okay
    synapse->cls = cls;
    synapse->weight = weight;
    synapse->conductance = 0.0f;
    
    synapse->n_max_spike_times = cls->delay + 1;
    synapse->n_spike_times = 0;
    synapse->spike_times_head = 0;
    synapse->spike_times_tail = 0;
    synapse->spike_times = (u32*)(synapse + 1);
    
    error:
    return;
}


internal u32
synapse_next_spike_time(Synapse* synapse) {
    check(synapse != NULL, "synapse is NULL");
    
    u32 result = synapse->spike_times[synapse->spike_times_head];
    return result;
    
    error:
    return 0;
}


internal void
synapse_add_spike_time(Synapse* synapse, u32 spike_time) {
    check(synapse != NULL, "synapse is NULL");
    
    // NOTE: add the time when the spike should be processed
    spike_time += synapse->cls->delay;
    
    // NOTE: current spike time should be larger than the one aldeady in the queue
    if_check(synapse->n_spike_times != 0,
             synapse_next_spike_time(synapse) < spike_time,
             "Spike %u should not be older than the head %u",
             spike_time,
             synapse_next_spike_time(synapse) < spike_time);
    
    synapse->spike_times[synapse->spike_times_tail] = spike_time;
    ++(synapse->n_spike_times);
    ++(synapse->spike_times_tail);
    if (synapse->spike_times_tail == synapse->n_max_spike_times)
        synapse->spike_times_tail = 0;
    
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
    
    if (synapse_is_spike_arriving_at_this_time(synapse, time)) {
        synapse->conductance += 1.0f;
        --(synapse->n_spike_times);
        ++(synapse->spike_times_head);
        if (synapse->spike_times_head == synapse->n_max_spike_times)
            synapse->spike_times_head = 0;
    } else {
        // NOTE: conductance should always be positive
        // NOTE: clip the conductance if its too low
        if (synapse->conductance > 0.001f) {
            synapse->conductance *= synapse->cls->tau_exp;
        } else {
            synapse->conductance = 0.0f;
        }
    }
    
    TIMING_COUNTER_END(SYNAPSE_STEP);
    
    error:
    return;
}


internal void
synapse_clear(Synapse* synapse) {
    check(synapse != NULL, "synapse is NULL");
    
    synapse->conductance = 0;
    synapse->n_spike_times = 0;
    synapse->spike_times_head = 0;
    synapse->spike_times_tail = 0;
    
    error:
    return;
}


internal f32
synapse_stdp_potentiation_weight_update(u32 interval) {
    return 0.1f;
}

internal void
synapse_stdp_potentiation_update(Synapse* synapse) {
    check(synapse != NULL, "synapse is NULL");

    u32 out_neuron_spike_time = synapse->out_neuron->last_spike_time;
    u32 in_neuron_spike_time = synapse->in_neuron->last_spike_time;

    // NOTE: the input neuron should have spiked
    if (in_neuron_spike_time == NEURON_INVALID_SPIKE_TIME) return;

    u32 interval_value = out_neuron_spike_time - in_neuron_spike_time;
    // NOTE: the interval between the spike should be big enough and it should not be 0
    if (interval_value > SYNAPSE_POTENTIATION_INTERVAL ||
        interval_value < synapse->cls->delay)
        return;

    synapse->weight += synapse_stdp_potentiation_weight_update(interval_value);
    // TODO: add an STDP Structure that the synapse class points to in the future
    if (synapse->weight > SYNAPSE_WEIGHT_MAX) {
        synapse->weight = SYNAPSE_WEIGHT_MAX;
    }

    error:
    return;
}


internal f32
synapse_stdp_depression_weight_update(u32 interval) {
    return -0.001f;
}

internal void
synapse_stdp_depression_update(Synapse* synapse) {
    check(synapse != NULL, "synapse is NULL");

    u32 out_neuron_spike_time = synapse->out_neuron->last_spike_time;
    u32 in_neuron_spike_time = synapse->in_neuron->last_spike_time;
    
    // NOTE: the out neuron should have spiked
    if (out_neuron_spike_time == NEURON_INVALID_SPIKE_TIME) return;

    u32 interval_value = in_neuron_spike_time - out_neuron_spike_time;
    // NOTE: the interval between the spike should be big enough and it should not be 0
    if (interval_value > SYNAPSE_DEPRESSION_INTERVAL ||
        interval_value == 0)
        return;

    synapse->weight += synapse_stdp_depression_weight_update(interval_value);
    if (synapse->weight > SYNAPSE_WEIGHT_MAX) {
        synapse->weight = SYNAPSE_WEIGHT_MAX;
    }

    error:
    return;
}

