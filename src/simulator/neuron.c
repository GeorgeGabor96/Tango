#include "simulator/neuron.h"


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
        log_error("INVALID neuron type %u", neuron->cls->type);
    }
    error:
    return spike;
}


// ACTUAL functions

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
neuron_create(NeuronClass* cls) {
    Neuron* neuron = NULL;
    check(cls != NULL, "cls is NULL");
    
    neuron = (Neuron*)memory_calloc(1, sizeof(Neuron),
                                    "neuron_create");
    
    if (neuron->cls->type == NEURON_LIF) {
        neuron->u = NEURON_LIF_U_REST;
    } else if (neuron->cls->type == NEURON_LIF_REFRACT) {
        neuron->u = NEURON_LIF_U_REST;
        neuron->data.lif_refract.last_spike_time = 0;
    }
    neuron->cls = cls;
    
    error:
    return neuron;
}


internal void
neuron_destroy(Neuron* neuron) {
    check(neuron != NULL, "neuron is NULL");
    
    memset(neuron, 0, sizeof(Neuron));
    memory_free(neuron);
    
    error:
    return;
}


internal void
neuron_move(Neuron* neuron_src, Neuron* neuron_dst) {
    check(neuron_src != NULL, "neuron_src is NULL");
    check(neuron_dst != NULL, "neuron_dst is NULL");
    
    memcpy(neuron_dst, neuron_src, sizeof(Neuron));
    error:
    return;
}
