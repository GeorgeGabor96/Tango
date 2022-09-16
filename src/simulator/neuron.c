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
internal NeuronClass*
neuron_class_create_lif() {
    NeuronClass* neuron_class = (NeuronClass*)memory_calloc(1, sizeof(NeuronClass), 
                                                            "neuron_class_create_lif");
    check_memory(neuron_class);
    neuron_class->type = NEURON_LIF_REFRACT;
    
    error:
    return neuron_class;
    
}

internal NeuronClass*
neuron_class_create_lif_refract(u32 refract_time) {
    NeuronClass* neuron_class = (NeuronClass*)memory_calloc(1, sizeof(NeuronClass), 
                                                            "neuron_class_create_lif_refract");
    check_memory(neuron_class);
    neuron_class->type = NEURON_LIF_REFRACT;
    neuron_class->data.lif_refract.refract_time = refract_time;
    
    error:
    return neuron_class;
    
}


internal void
neuron_class_destroy(NeuronClass* cls) {
    check(cls != NULL, "cls is NULL");
    
    memset(cls, 0, sizeof(NeuronClass));
    memory_free(cls);
    
    error:
    return;
}


internal void
neuron_class_move(NeuronClass* cls_src, NeuronClass* cls_dst) {
    check(cls_src != NULL, "cls_src is NULL");
    check(cls_dst != NULL, "cls_dst is NULL");
    
    memcpy(cls_dst, cls_src, sizeof(NeuronClass));
    error:
    return;
}


/********************
*   NEURON
********************/
internal Neuron*
neuron_create(NeuronClass* cls, Array* in_synapses_ref, Array* out_synapses_ref) {
    Neuron* neuron = NULL;
    check(cls != NULL, "cls is NULL");
    check(in_synapses_ref != NULL, "in_synapses_ref is NULL");
    check(out_synapses_ref != NULL, "out_synapses_ref is NULL");
    
    neuron = (Neuron*)memory_calloc(1, sizeof(Neuron),
                                    "neuron_create");
    neuron->cls = cls;
    neuron->epsc = 0.0f;
    neuron->ipsc = 0.0f;
    neuron->in_synapses_ref = in_synapses_ref;
    neuron->out_synapses_ref = out_synapses_ref;
    neuron->spike = FALSE;
    
    if (neuron->cls->type == NEURON_LIF) {
        neuron->u = NEURON_LIF_U_REST;
    } else if (neuron->cls->type == NEURON_LIF_REFRACT) {
        neuron->u = NEURON_LIF_U_REST;
        neuron->data.lif_refract.last_spike_time = 0;
    }
    
    
    error:
    return neuron;
}


internal void
neuron_destroy(Neuron* neuron) {
    check(neuron != NULL, "neuron is NULL");
    
    array_destroy(neuron->in_synapses_ref, NULL);
    array_destroy(neuron->out_synapses_ref, NULL);
    memset(neuron, 0, sizeof(Neuron));
    memory_free(neuron);
    
    error:
    return;
}


// TODO:
/*
internal void
neuron_move(Neuron* neuron_src, Neuron* neuron_dst) {
    check(neuron_src != NULL, "neuron_src is NULL");
    check(neuron_dst != NULL, "neuron_dst is NULL");
    
    memcpy(neuron_dst, neuron_src, sizeof(Neuron));
    error:
    return;
}
*/


inline internal f32
neuron_compute_psc(neuron, time) {
    f32 epsc = 0.0f;
    f32 ipsc = 0.0f;
    f32 current = 0.0f;
    u32 i = 0;
    Synapse* synapse = NULL;
    
    for (i = 0u; i < neuron->in_synapses_ref->length; ++i) {
        synapse = *((Synapse**)array_get(neuron->in_synapses_ref, i));
        current = synapse_compute_psc(synapse, neuron->voltage);
        
        if (current >= 0) epsc += current;
        else ipsc += current;
        
        // TODO: why step after psc?
        synapse_step(synapse, time);
    }
    
    neuron->epsc = epsc;
    neuron->ipsc = ipsc;
    return epsc + ipsc;
}


inline internal void
neuron_lif_u_update(Neuron* neuron) {
    neuron->u = NEURON_LIF_U_FACTOR * neuron->u + 
        NEURON_LIF_I_FACTOR * psc + 
        NEURON_LIF_FREE_FACTOR;
}


inline internal void
neuron_update(Neuron* neuron, u32 time, f32 psc) {
    bool spike = FALSE;
    
    NeuronCls cls = neuron->cls;
    
    if (neuron->cls->type == NEURON_LIF) {
        neuron_lif_u_update(neuron);
        if (neuron->u >= NEURON_LIF_U_TH) {
            neuron->u = NEURON_LIF_U_REST;
            spike = TRUE;
        }
    } else if (neuron->cls->type == NEURON_LIF_REFRACT) {
        // NOTE: Only update after refractory period
        if (time - neuron->data.last_spike_time <= cls->data.refractory_time) {
            neuron->u = NEURON_LIF_U_REST;
            spike = FALSE;
        } else {
            neuron_lif_u_update(neuron);
            if (neuron->u >= NEURON_LIF_U_TH) {
                neuron->u = NEURON_LIF_U_REST;
                neuron->last_spike_time = time;
                spike = TRUE;
            }
        }
    } else {
        log_error("INVALID neuron type %u", cls->type);
    }
    error:
    return spike;
}


inline internal void
neuron_update_in_synapses(Neuron* neuron, u32 time) {
    u32 i = 0;
    Synapse* synapse = NULL;
    for (i = 0u; i < neuron->in_synapses_ref; ++i) {
        synapse = *((Synapse**)array_get(neuron->in_synapses_ref, i));
        synapse_step(synapse, time);
    }
}


inline internal void
neuron_update_out_synapses(Neuron* neuron, u32 time) {
    u32 i = 0;
    Synapse* synapse = NULL;
    
    if (neuron->spike == TRUE) {
        for (i = 0u; i < neuron->out_synapses_ref; ++i) {
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
        neuron->voltage = NEURON_LIF_U_REST;
    } else if (neuron->cls->type == NEURON_LIF_REFRACT) {
        neuron->u = NEURON_LIF_U_REST;
        neuron->last_spike_time = time;
    } else {
        log_error("INVALID neuron type %u", cls->type);
    }
    neuron_update_out_synapses(neuron, time);
    
    error:
    return;
}


internal void
neuron_step_inject_current(Neuron* neuron, f32 psc, u32 time) {
    check(neuron != NULL, "neuron is NULL");
    
    f32 psc = neuron_compute_psc(neuron, time) + psc;
    neuron_update(neuron, time, psc);
    neuron_update_out_synapses(neuron, time);
    
    error:
    return;
}