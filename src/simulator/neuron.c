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
neuron_cls_create_lif(String* name) {
    NeuronCls* neuron_cls = NULL;
    check(name != NULL, "name is NULL");
    neuron_cls = (NeuronCls*)memory_calloc(1, sizeof(*neuron_cls), 
                                           "neuron_class_create_lif");
    check_memory(neuron_cls);
    neuron_cls->name = name;
    neuron_cls->type = NEURON_LIF_REFRACT;
    
    error:
    return neuron_cls;
}


internal NeuronCls*
neuron_cls_create_lif_refract(String* name, u32 refract_time) {
    NeuronCls* neuron_cls = NULL;
    check(name != NULL, "name is NULL");
    neuron_cls = (NeuronCls*)memory_calloc(1, sizeof(NeuronCls), 
                                           "neuron_class_create_lif_refract");
    check_memory(neuron_cls);
    neuron_cls->name = name;
    neuron_cls->type = NEURON_LIF_REFRACT;
    neuron_cls->lif_refract_cls.refract_time = refract_time;
    
    error:
    return neuron_cls;
}


internal void
neuron_cls_destroy(NeuronCls* cls) {
    check(cls != NULL, "cls is NULL");
    
    string_destroy(cls->name);
    memset(cls, 0, sizeof(*cls));
    memory_free(cls);
    
    error:
    return;
}


internal void
neuron_cls_move(NeuronCls* cls_src, NeuronCls* cls_dst) {
    check(cls_src != NULL, "cls_src is NULL");
    check(cls_dst != NULL, "cls_dst is NULL");
    
    memcpy(cls_dst, cls_src, sizeof(*cls_src));
    memset(cls_src, 0, sizeof(*cls_src));
    
    error:
    return;
}


/********************
*   NEURON
********************/
#define NEURON_INITIAL_N_IN_SYNAPSES 10
#define NEURON_INITIAL_N_OUT_SYNAPSES 10
#define NEURON_SYNAPSE_INCREASE_FACTOR 2


internal Neuron*
neuron_create(NeuronCls* cls) {
    Neuron* neuron = NULL;
    check(cls != NULL, "cls is NULL");
    neuron = (Neuron*)memory_calloc(1, sizeof(Neuron),
                                    "neuron_create");
    check_memory(neuron);
    bool status = neuron_init(neuron, cls);
    check(status == TRUE, "couldn't init the neuron");
    
    return neuron;
    
    error:
    memory_free(neuron);
    
    return NULL;
}


internal bool
neuron_init(Neuron* neuron, NeuronCls* cls) {
    check(neuron != NULL, "neuron is NULL");
    check(cls != NULL, "cls is NULL");
    
    neuron->in_synapses = (Synapse*)memory_calloc(NEURON_INITIAL_N_IN_SYNAPSES,
                                                  sizeof(Synapse),
                                                  "neuron_init in_synapses");
    check_memory(neuron->in_synapses);
    neuron->n_in_synapses = 0;
    neuron->n_max_in_synapses = NEURON_INITIAL_N_IN_SYNAPSES;
    
    neuron->out_p_synapses = (SynapseP*)memory_calloc(NEURON_INITIAL_N_OUT_SYNAPSES,
                                                      sizeof(SynapseP),
                                                      "neuron_init out_synapses");
    check_memory(neuron->out_p_synapses);
    neuron->n_out_synapses = 0;
    neuron->n_max_out_synapses = NEURON_INITIAL_N_OUT_SYNAPSES;
    
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
    
    return TRUE;
    
    error:
    if (neuron->in_synapses != NULL)
        memory_free(neuron->in_synapses);
    
    if (neuron->out_p_synapses != NULL)
        memory_free(neuron->out_p_synapses);
    memset(neuron, 0, sizeof(*neuron));
    return FALSE;
}


internal void
neuron_destroy(Neuron* neuron) {
    check(neuron != NULL, "neuron is NULL");
    
    neuron_reset(neuron);
    memory_free(neuron);
    
    error:
    return;
}


internal void
neuron_reset(Neuron* neuron) {
    check(neuron != NULL, "neuron is NULL");
    u32 i = 0;
    
    // NOTE: neuron owns input synapses
    for (i = 0; i < neuron->n_in_synapses; ++i)
        synapse_reset(neuron->in_synapses + i);
    
    memory_free(neuron->in_synapses);
    memory_free(neuron->out_p_synapses);
    
    memset(neuron, 0, sizeof(*neuron));
    
    error:
    return;
}


internal Synapse*
neuron_add_in_synapse(Neuron* neuron, Synapse* synapse, bool free_synapse) {
    check(neuron != NULL, "neuron is NULL");
    check(synapse != NULL, "synapse is NULL");
    
    // NOTE: move the synapse into the in_synapses_ref
    if (neuron->n_in_synapses == neuron->n_max_in_synapses) {
        u32 new_length = neuron->n_in_synapses * NEURON_SYNAPSE_INCREASE_FACTOR; 
        neuron->in_synapses = array_resize(neuron->in_synapses,
                                           sizeof(Synapse),
                                           neuron->n_in_synapses,
                                           new_length);
        neuron->n_max_in_synapses = new_length;
    }
    
    memcpy(neuron->in_synapses + neuron->n_in_synapses, synapse, sizeof(*synapse));
    memset(synapse, 0, sizeof(*synapse));
    if (free_synapse) memory_free(synapse);
    
    synapse = neuron->in_synapses + neuron->n_in_synapses;
    ++(neuron->n_in_synapses);
    return synapse;
    
    error:
    return NULL;
}


internal void
neuron_add_out_synapse(Neuron* neuron, Synapse* synapse) {
    check(neuron != NULL, "neuron is NULL");
    check(synapse != NULL, "synapse is NULL");
    
    if (neuron->n_out_synapses == neuron->n_max_out_synapses) {
        u32 new_length = neuron->n_out_synapses * NEURON_SYNAPSE_INCREASE_FACTOR;
        neuron->out_p_synapses = array_resize(neuron->out_p_synapses,
                                              sizeof(SynapseP),
                                              neuron->n_out_synapses,
                                              new_length);
        neuron->n_max_out_synapses = new_length;
    }
    
    neuron->out_p_synapses[neuron->n_out_synapses] = synapse;
    ++(neuron->n_out_synapses);
    
    error:
    return;
}


inline internal f32
neuron_compute_psc(Neuron* neuron, u32 time) {
    f32 epsc = 0.0f;
    f32 ipsc = 0.0f;
    f32 current = 0.0f;
    u32 i = 0;
    Synapse* synapse = NULL;
    
    for (i = 0; i < neuron->n_in_synapses; ++i) {
        synapse = neuron->in_synapses + i;
        // NOTE: first update the synapse for the case where there is a spike with the 
        // NOTE: current time
        // NOTE: We could also step the synapse after the psc computation
        // NOTE: but the effect will be applied only after at the next step
        // NOTE: For now I think this is the more correct and easy to understand
        synapse_step(synapse, time);
        current = synapse_compute_psc(synapse, neuron->voltage);
        
        if (current >= 0) epsc += current;
        else ipsc += current;
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
    Synapse* synapse = NULL;
    for (i = 0; i < neuron->n_in_synapses; ++i) {
        synapse_step(neuron->in_synapses + i, time);
    }
}


inline internal void
neuron_update_out_synapses(Neuron* neuron, u32 time) {
    u32 i = 0;
    Synapse* synapse = NULL;
    
    if (neuron->spike == TRUE) {
        for (i = 0; i < neuron->n_out_synapses; ++i) {
            synapse_add_spike_time(neuron->out_p_synapses[i], time);
        }
    }
}


internal void
neuron_step(Neuron* neuron, u32 time) {
    check(neuron != NULL, "neuron is NULL");
    
    f32 psc = neuron_compute_psc(neuron, time);
    neuron_update(neuron, time, psc);
    neuron_update_out_synapses(neuron, time);
    
    error:
    return;
}


internal void
neuron_step_force_spike(Neuron* neuron, u32 time) {
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
    
    error:
    return;
}


internal void
neuron_step_inject_current(Neuron* neuron, f32 psc, u32 time) {
    check(neuron != NULL, "neuron is NULL");
    
    psc = neuron_compute_psc(neuron, time) + psc;
    neuron_update(neuron, time, psc);
    neuron_update_out_synapses(neuron, time);
    
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
    for (u32 i = 0; i < neuron->n_in_synapses; ++i)
        synapse_clear(neuron->in_synapses + i);
    
    error:
    return;
}
