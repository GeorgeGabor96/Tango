#include "simulator/neuron.h"


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
    neuron_cls = (NeuronCls*)memory_arena_push(state->permanent_storage, sizeof(*neuron_cls));
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
    neuron_cls = (NeuronCls*)memory_arena_push(state->permanent_storage, sizeof(NeuronCls));
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
    neuron = (Neuron*)memory_arena_push(state->permanent_storage, sizeof(Neuron));
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
    
    memset(neuron->in_synapse_arrays, 0, sizeof(InSynapseArray*) * NEURON_N_MAX_INPUTS);
    memset(neuron->out_synapse_arrays, 0, sizeof(OutSynapseArray*) * NEURON_N_MAX_OUTPUTS);
    
    neuron->n_in_synapse_arrays = 0;
    neuron->n_out_synapse_arrays = 0;
    
    neuron->cls = cls;
    neuron->epsc = 0.0f;
    neuron->ipsc = 0.0f;
    neuron->spike = FALSE;
    
    if (neuron->cls->type == NEURON_LIF) {
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
    } else if (neuron->cls->type == NEURON_LIF_REFRACT) {
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
        neuron->lif_refract.last_spike_time = 0;
    }
    
    error:
    return;
}


internal void 
neuron_add_in_synapse_array(Neuron* neuron, InSynapseArray* synapses) {
    check(neuron != NULL, "neuron is NULL");
    check(synapses != NULL, "synapses is NULL");
    
    neuron->in_synapse_arrays[neuron->n_in_synapse_arrays] = synapses;
    ++(neuron->n_in_synapse_arrays);
    
    error:
    return;
}


internal void
neuron_add_out_synapse_array(Neuron* neuron, OutSynapseArray* synapses) {
    check(neuron != NULL, "neuron is NULL");
    check(synapses != NULL, "synapses is NULL");
    
    neuron->out_synapse_arrays[neuron->n_out_synapse_arrays] = synapses;
    ++(neuron->n_out_synapse_arrays);
    
    error:
    return;
}


inline internal f32
neuron_compute_psc(Neuron* neuron, u32 time) {
    f32 epsc = 0.0f;
    f32 ipsc = 0.0f;
    f32 current = 0.0f;
    u32 i = 0;
    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    InSynapseArray* synapses = NULL;
    
    for (i = 0; i < neuron->n_in_synapse_arrays; ++i) {
        synapses = neuron->in_synapse_arrays[i];
        
        for (synapse_i = 0; synapse_i < synapses->length; ++synapse_i) {
            synapse = (Synapse*)
            ((u8*)(synapses->synapses) + synapses->synapse_size * synapse_i);
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


inline internal void
neuron_lif_voltage_update(Neuron* neuron, f32 psc) {
    neuron->voltage = NEURON_LIF_VOLTAGE_FACTOR * neuron->voltage + 
        NEURON_LIF_I_FACTOR * psc + 
        NEURON_LIF_FREE_FACTOR;
}


inline internal void
neuron_update(Neuron* neuron, u32 time, f32 psc) {
    bool spike = FALSE;
    
    NeuronCls* cls = neuron->cls;
    
    if (neuron->cls->type == NEURON_LIF) {
        neuron_lif_voltage_update(neuron, psc);
        if (neuron->voltage >= NEURON_LIF_VOLTAGE_TH) {
            neuron->voltage = NEURON_LIF_VOLTAGE_REST;
            spike = TRUE;
        }
    } else if (neuron->cls->type == NEURON_LIF_REFRACT) {
        // NOTE: Only update after refractory period
        if (time - neuron->lif_refract.last_spike_time <= cls->lif_refract_cls.refract_time) {
            neuron->voltage = NEURON_LIF_VOLTAGE_REST;
            spike = FALSE;
        } else {
            neuron_lif_voltage_update(neuron, psc);
            if (neuron->voltage >= NEURON_LIF_VOLTAGE_TH) {
                neuron->voltage = NEURON_LIF_VOLTAGE_REST;
                neuron->lif_refract.last_spike_time = time;
                spike = TRUE;
            }
        }
    } else {
        log_error("INVALID neuron type %u", cls->type);
    }
    neuron->spike = spike;
}


inline internal void
neuron_update_in_synapses(Neuron* neuron, u32 time) {
    u32 i = 0;
    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    InSynapseArray* synapses = NULL;
    for (i = 0; i < neuron->n_in_synapse_arrays; ++i) {
        synapses = neuron->in_synapse_arrays[i];
        
        for (synapse_i = 0; synapse_i < synapses->length; ++synapse_i) {
            synapse = (Synapse*)
            ((u8*)(synapses->synapses) + synapses->synapse_size * synapse_i);
            synapse_step(synapse, time);
        }
    }
}


inline internal void
neuron_update_out_synapses(Neuron* neuron, u32 time) {
    u32 i = 0;
    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    OutSynapseArray* synapses = NULL;
    
    if (neuron->spike == FALSE) return;
    
    for (i = 0; i < neuron->n_out_synapse_arrays; ++i) {
        synapses = neuron->out_synapse_arrays[i];
        
        for (synapse_i = 0; synapse_i < synapses->length; ++synapse_i) {
            synapse = synapses->synapses[synapse_i];
            synapse_add_spike_time(synapse, time);
        }
    }
}


internal void
neuron_step(Neuron* neuron, u32 time) {
    TIMING_COUNTER_START(NEURON_STEP);
    
    check(neuron != NULL, "neuron is NULL");
    
    f32 psc = neuron_compute_psc(neuron, time);
    neuron_update(neuron, time, psc);
    neuron_update_out_synapses(neuron, time);
    
    TIMING_COUNTER_END(NEURON_STEP);
    
    error:
    return;
}


internal void
neuron_step_force_spike(Neuron* neuron, u32 time) {
    TIMING_COUNTER_START(NEURON_STEP_FORCE_SPIKE);
    
    check(neuron != NULL, "neuron is NULL");
    
    neuron_update_in_synapses(neuron, time);
    neuron->spike = TRUE;
    
    if (neuron->cls->type == NEURON_LIF) {
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
    } else if (neuron->cls->type == NEURON_LIF_REFRACT) {
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
        neuron->lif_refract.last_spike_time = time;
    } else {
        log_error("INVALID neuron type %u", neuron->cls->type);
    }
    neuron_update_out_synapses(neuron, time);
    
    TIMING_COUNTER_END(NEURON_STEP_FORCE_SPIKE);
    
    error:
    return;
}


internal void
neuron_step_inject_current(Neuron* neuron, f32 psc, u32 time) {
    TIMING_COUNTER_START(NEURON_STEP_INJECT_CURRENT);
    
    check(neuron != NULL, "neuron is NULL");
    
    psc = neuron_compute_psc(neuron, time) + psc;
    neuron_update(neuron, time, psc);
    neuron_update_out_synapses(neuron, time);
    
    TIMING_COUNTER_END(NEURON_STEP_INJECT_CURRENT);
    
    error:
    return;
}


internal void
neuron_clear(Neuron* neuron) {
    check(neuron != NULL, "neuron is NULL");
    
    neuron->spike = FALSE;
    if (neuron->cls->type == NEURON_LIF)
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
    else if (neuron->cls->type == NEURON_LIF_REFRACT)
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
    else 
        log_error("Unknown neuron type %u", neuron->cls->type);
    
    Synapse* synapse = NULL;
    InSynapseArray* synapses = NULL;
    u32 i = 0;
    u32 synapse_i = 0;
    for (i = 0; i < neuron->n_in_synapse_arrays; ++i) {
        synapses = neuron->in_synapse_arrays[i];
        for (synapse_i = 0; synapse_i < synapses->length; ++synapse_i) {
            synapse = (Synapse*)
            ((u8*)(synapses->synapses) + synapses->synapse_size * synapse_i);
            synapse_clear(synapse);
        }
    }
    
    error:
    return;
}
