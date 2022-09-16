#include "simulator/synapse.h"

/*******************
* Helpfull functions
*******************/
inline internal bool
synapse_is_spike_arriving_at_this_time(Queue* spike_times, u32 time) {
    bool result = (queue_is_empty(spike_times) == FALSE) && (queue_head(spike_times) == time);
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
synapse_cls_create_conductance(f32 rev_pot, f32 amp, f32 tau_ms, u32 delay) {
    check(rev_pot > 0.0f, "rev_pot needs to be > 0");
    check(amp > 0.0f, "amp needs to be > 0");
    check(tau_ms > 0.0f, "tau_ms needs to be > 0");
    // TODO: Should I check for the delay??
    
    SynapseClass* cls = (SynapseCls*) memory_malloc(sizeof(SyanpseCls));
    check_memory(cls);
    
    cls->rev_pot = rev_pot;
    cls->amp = amp;
    cls->tau = (f32)math_op_exp((f64)(-1)/tau_ms);
    cls->delay = delay;
    cls->type = SYNAPSE_CONDUCTANCE;
}


/*******************
* Synapse
*******************/
internal Synapse*
synapse_create(SynapseCls* cls, f32 weight) {
    Synapse* synapse = NULL;
    check(cls != NULL, "cls is NULL");
    
    synapse = (Synapse*)memory_malloc(sizeof(Synapse));
    check_memory(synapse);
    
    // TODO: Depending on the order of update and add spike time
    // TODO: you need a number of delay slots in the queue (in case update then add)
    // TODO: or delay + 1 (in case add then update)
    // TODO: Recheck this, for now its okay
    synapse->spike_times = queue_create(cls->delay + 1, sizeof(u32));
    check_memory(synapse->spike_times);
    synapse->cls = cls;
    synapse->weight = weight;
    synapse->conductance = 0.0f;
    
    error:
    if (synapse != NULL) memory_free(synapse);
    return synapse;
}


internal void
synapse_destroy(Synapse* synapse) {
    check(synapse != NULL, "synapse is NULL");
    
    queue_destroy(synapse->spike_times);
    memset(synapse, 0, sizeof(Synapse));
    memory_free(synapse);
    
    error:
    return;
}


internal void
synapse_add_spike_time(Synapse* synapse, u32 spike_time) {
    check(synapse != NULL, "synapse is NULL");
    
    // NOTE: add the time when the spike should be processed
    spike_time += synapse->cls->delay;
    
    // NOTE: current spike time should be larger than the one aldeady in the queue
    if_check(queue_is_empty(synapse->spike_times) == FALSE,
             *((u32*)queue_head(synapse->spike_times)) < spike_time,
             "Spike %u should not be older than the head %u",
             spike_time,
             *((u32*)queue_head(synapse->spike_times)) < spike_time);
    
    queue_enqueue(synapse->spike_times, &spike_time);
    
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
        I = cls->amp * synapse->weight * synapse->conductance;
    } else if (cls->type == SYNAPSE_VOLTAGE) {
        I = - cls->amp * synapse->weight * synapse->conductance 
            * (voltage - cls->rev_potential);
    } else {
        log_error("INVALID synapse type %u", cls->type);
    }
    
    error:
    return current;
}


internal void
synapse_step(Synapse* synapse, u32 time) {
    check(synapse != NULL, "synapse is NULL");
    
    if (synapse_is_spike_arriving_at_this_time(synapse->spike_times, time)) {
        synapse->conductance += 1.0f;
        queue_dequeue(synapse->spike_times));
    } else {
        // NOTE: conductance should always be positive
        // NOTE: clip the conductance if its too low
        if (synapse->conductance > 0.001f) {
            synapse->conductance *= synapse->cls->tau_exp;
        } else {
            synapse->conductance = 0.0f;
        }
    }
    
    error:
    return;
}
