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
internal Neuron*
neuron_create(NeuronCls* cls) {
    Neuron* neuron = NULL;
    check(cls != NULL, "cls is NULL");
    neuron = (Neuron*)memory_calloc(1, sizeof(Neuron),
                                    "neuron_create");
    check_memory(neuron);
    neuron_init(neuron, cls);
    
    error:
    return neuron;
}


internal void
neuron_init(Neuron* neuron, NeuronCls* cls) {
    check(neuron != NULL, "neuron is NULL");
    check(cls != NULL, "cls is NULL");
    
    neuron->cls = cls;
    neuron->epsc = 0.0f;
    neuron->ipsc = 0.0f;
    neuron->in_synapses_ref = NULL;
    neuron->out_synapses_ref = NULL;
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
    
    // TODO: how deletes the synapses????
    memset(neuron, 0, sizeof(Neuron));
    
    error:
    return;
}


internal void
neuron_reset_double_p(Neuron** neuron) {
    check(neuron != NULL, "neuron is NULL");
    
    neuron_reset(*neuron);
    
    error:
    return;
}


internal void
neuron_add_in_synapses_ref(Neuron* neuron, Array* synapses_ref) {
    check(neuron != NULL, "neuron is NULL");
    check(synapses_ref != NULL, "synapses_ref is NULL");
    check(synapses_ref->el_size == sizeof(void*),
          "the synapses_ref should contain pointers");
    check(neuron->in_synapses_ref == NULL, "input synapses already added");
    
    neuron->in_synapses_ref = synapses_ref;
    
    error:
    return;
}


internal void
neuron_add_out_synapses_ref(Neuron* neuron, Array* synapses_ref) {
    check(neuron != NULL, "neuron is NULL");
    check(synapses_ref != NULL, "synapses_ref is NULL");
    check(synapses_ref->el_size == sizeof(void*),
          "the synapses_ref should contain pointers");
    check(neuron->out_synapses_ref == NULL, "output synapses already added");
    
    neuron->out_synapses_ref = synapses_ref;
    
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
    
    for (i = 0u; i < neuron->in_synapses_ref->length; ++i) {
        synapse = *((Synapse**)array_get(neuron->in_synapses_ref, i));
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
    for (i = 0u; i < neuron->in_synapses_ref->length; ++i) {
        synapse = *((Synapse**)array_get(neuron->in_synapses_ref, i));
        synapse_step(synapse, time);
    }
}


inline internal void
neuron_update_out_synapses(Neuron* neuron, u32 time) {
    u32 i = 0;
    Synapse* synapse = NULL;
    
    if (neuron->spike == TRUE) {
        for (i = 0u; i < neuron->out_synapses_ref->length; ++i) {
            synapse = *((Synapse**)array_get(neuron->out_synapses_ref, i));
            synapse_add_spike_time(synapse, time);
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