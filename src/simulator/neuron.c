#include "common.h"
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