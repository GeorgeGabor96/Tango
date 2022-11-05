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
synapse_cls_create(const char* name, SynapseType type,
                   f32 rev_potential, f32 amp, f32 tau_ms, u32 delay) {
    SynapseCls* cls = NULL;
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
    
    cls = (SynapseCls*)memory_malloc(sizeof(*cls), "synapse_cls_create");
    check_memory(cls);
    
    cls->name = string_create(name);
    check_memory(cls->name);
    cls->type = type;
    cls->rev_potential = rev_potential;
    cls->amp = amp;
    cls->tau_exp = math_op_exp((f64)(-1)/tau_ms);
    cls->delay = delay;
    return cls;
    
    error:
    if (cls != NULL) {
        if (cls->name != NULL) string_destroy(cls->name);
        memory_free(cls);
    }
    return NULL;
}


internal void
synapse_cls_destroy(SynapseCls* cls) {
    check(cls != NULL, "cls is NULL");
    
    synapse_cls_reset(cls);
    memory_free(cls);
    
    error:
    return;
}


internal void
synapse_cls_reset(SynapseCls* cls) {
    check(cls != NULL, "cls is NULL");
    
    string_destroy(cls->name);
    memset(cls, 0, sizeof(*cls));
    
    error:
    return;
}


internal void
synapse_cls_move(SynapseCls* cls_src, SynapseCls* cls_dst) {
    check(cls_src != NULL, "cls_src is NULL");
    check(cls_dst != NULL, "cls_dst is NULL");
    
    memcpy(cls_dst, cls_src, sizeof(*cls_src));
    memset(cls_src, 0, sizeof(*cls_src));
    
    error:
    return;
}


/*******************
* Synapse
*******************/
internal Synapse*
synapse_create(SynapseCls* cls, f32 weight) {
    Synapse* synapse = NULL;
    
    synapse = (Synapse*)memory_malloc(sizeof(*synapse), "synapse_create");
    check_memory(synapse);
    
    bool status = synapse_init(synapse, cls, weight);
    check(status == TRUE, "couldn't init synapse");
    
    return synapse;
    
    error:
    if (synapse != NULL) {
        memset(synapse, 0, sizeof(*synapse));
        memory_free(synapse);
    }
    return NULL;
}


internal bool
synapse_init(Synapse* synapse, SynapseCls* cls, f32 weight) {
    check(synapse != NULL, "synapse is NULL");
    check(cls != NULL, "cls is NULL");
    check(weight >= 0.0f, "weight is negative");
    
    // NOTE: Depending on the order of update and add spike time
    // NOTE: you 
    // need a number of delay slots in the queue (in case update then add)
    // NOTE: or delay + 1 (in case add then update)
    // NOTE: Recheck this, for now its okay
    synapse->spike_times = (u32*)memory_malloc((cls->delay + 1) * sizeof(u32),
                                               "synapse_init");
    check_memory(synapse->spike_times);
    synapse->spike_times_head = 0;
    synapse->spike_times_tail = 0;
    synapse->n_spike_times = 0;
    synapse->n_max_spike_times = cls->delay + 1;
    
    synapse->cls = cls;
    synapse->weight = weight;
    synapse->conductance = 0.0f;
    return TRUE;
    
    error:
    return FALSE;
    
}


internal void
synapse_destroy(Synapse* synapse) {
    check(synapse != NULL, "synapse is NULL");
    memory_free(synapse);
    
    synapse_reset(synapse);
    
    error:
    return;
}


internal void
synapse_reset(Synapse* synapse) {
    check(synapse != NULL, "synapse is NULL");
    
    memory_free(synapse->spike_times);
    memset(synapse, 0, sizeof(Synapse));
    
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
    synapse->spike_times_head = 0;
    synapse->spike_times_tail = 0;
    
    error:
    return;
}