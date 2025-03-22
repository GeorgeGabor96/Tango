/****************************
*   NEURON SYNAPSE ARRAY IDXS
****************************/
SynapseRefArray* neuron_create_synapses_ref_array(Memory* memory, u32 capacity) {
    check(memory != NULL, "memory is NULL");
    check(capacity != 0, "capacity is 0");

    SynapseRefArray* array = memory_push(memory, sizeof(*array) * capacity * sizeof(SynapseP));
    check_memory(array);

    array->capacity = capacity;
    array->length = 0;
    array->synapses = (SynapseP*)(array + 1);

    return array;
    error:
    return NULL;
}



/********************
*   NEURON TYPE
********************/
internal char*
neuron_type_get_c_str(NeuronType type) {
    if (type == NEURON_IF) return "NEURON_IF";
    if (type == NEURON_IF_REFRACT) return "NEURON_IF_REFRACT";
    if (type == NEURON_IF_ONE_SPIKE) return "NEURON_IF_ONE_SPIKE";
    if (type == NEURON_LIF) return "NEURON_LIF";
    if (type == NEURON_LIF_REFRACT) return "NEURON_LIF_REFRACT";
    else return "NEURON_INVALID";
}


/********************
*   NEURON CLASS
********************/
inline internal NeuronCls*
_neuron_cls_create(Memory* memory, String* name) {
    check(memory != NULL, "memory is NULL");
    check(name != NULL, "name is NULL");

    NeuronCls* neuron_cls = (NeuronCls*)memory_push(memory, sizeof(*neuron_cls));
    check_memory(neuron_cls);

    neuron_cls->name = name;
    neuron_cls->type = NEURON_INVALID;

    return neuron_cls;

    error:
    return NULL;
}


internal NeuronCls*
neuron_cls_create_if(Memory* memory, String* name) {
    NeuronCls* n_cls = _neuron_cls_create(memory, name);
    check_memory(n_cls);

    n_cls->type = NEURON_IF;
    error:
    return n_cls;
}


internal NeuronCls*
neuron_cls_create_if_refract(Memory* memory, String* name, u32 refract_time) {
    check(refract_time > 0, "refract_time is 0");
    NeuronCls* n_cls = _neuron_cls_create(memory, name);
    check_memory(n_cls);

    n_cls->type = NEURON_IF_REFRACT;
    n_cls->if_refract_cls.refract_time = refract_time;
    return n_cls;

    error:
    return NULL;
}


internal NeuronCls* neuron_cls_create_if_one_spike(Memory* memory, String* name) {
    NeuronCls* n_cls = _neuron_cls_create(memory, name);
    check_memory(n_cls);

    n_cls->type = NEURON_IF_ONE_SPIKE;
    error:
    return n_cls;
}


internal NeuronCls*
neuron_cls_create_lif(Memory* memory, String* name) {
    NeuronCls* n_cls = _neuron_cls_create(memory, name);
    check_memory(n_cls);

    n_cls->type = NEURON_LIF;
    error:
    return n_cls;
}


internal NeuronCls*
neuron_cls_create_lif_refract(Memory* memory, String* name, u32 refract_time) {
    check(refract_time > 0, "refract_time is 0");
    NeuronCls* n_cls = _neuron_cls_create(memory, name);
    check_memory(n_cls);

    n_cls->type = NEURON_LIF_REFRACT;
    n_cls->lif_refract_cls.refract_time = refract_time;
    return n_cls;

    error:
    return NULL;
}


/********************
*   NEURON
********************/
internal Neuron*
neuron_create(Memory* memory, NeuronCls* cls) {
    check(memory != NULL, "memory is NULL");
    check(cls != NULL, "cls is NULL");
    Neuron* neuron = (Neuron*)memory_push(memory, sizeof(Neuron));
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

    neuron->cls = cls;
    neuron->voltage = NEURON_VOLTAGE_REST[neuron->cls->type];

    neuron->epsc = 0.0f;
    neuron->ipsc = 0.0f;
    neuron->spike = FALSE;
    neuron->last_spike_time = INVALID_SPIKE_TIME;

    neuron->in_synapse_arrays = NULL;
    neuron->out_synapse_arrays = NULL;
    neuron->n_in_synapse_arrays = 0;
    neuron->n_out_synapse_arrays = 0;

    error:
    return;
}


internal f32
_neuron_compute_psc(Neuron* neuron, u32 time) {
    f32 epsc = 0.0f;
    f32 ipsc = 0.0f;
    f32 current = 0.0f;
    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    SynapseRefArray* it = NULL;

    for (it = neuron->in_synapse_arrays; it != NULL; it = it->next) {
        for (synapse_i = 0; synapse_i < it->length; ++synapse_i) {
            synapse = it->synapses[synapse_i];

            current = synapse_compute_psc(synapse, neuron->voltage);

            // NOTE: update the synapse after we interogate it, to simulate a 1ms dendritic delay?
            synapse_step(synapse, time);

            if (current >= 0) epsc += current;
            else ipsc += current;
        }
    }

    neuron->epsc = epsc;
    neuron->ipsc = ipsc;
    return epsc + ipsc;
}


/****************************
 * NOTE: Neuron Update stuff
 ***************************/
#define _NEURON_IF_VOLTAGE_UPDATE(neuron, psc) (neuron->voltage = neuron->voltage + psc)
#define _NEURON_LIF_VOLTAGE_UPDATE(neuron, psc) (neuron->voltage = NEURON_LIF_VOLTAGE_FACTOR * neuron->voltage + NEURON_LIF_I_FACTOR * psc + NEURON_LIF_FREE_FACTOR)

internal void
_neuron_if_voltage_update(Neuron* neuron, f32 psc, u32 time) {
    _NEURON_IF_VOLTAGE_UPDATE(neuron, psc);
}

internal void
_neuron_if_refract_voltage_update(Neuron* neuron, f32 psc, u32 time) {
    if (time - neuron->last_spike_time <= neuron->cls->if_refract_cls.refract_time) {
        neuron->voltage = NEURON_IF_VOLTAGE_REST;
    } else {
        _NEURON_IF_VOLTAGE_UPDATE(neuron, psc);
    }
}

internal void
_neuron_if_one_spike_voltage_update(Neuron* neuron, f32 psc, u32 time) {
    if (neuron->last_spike_time == INVALID_SPIKE_TIME) {
        _NEURON_IF_VOLTAGE_UPDATE(neuron, psc);
    }
}

internal void
_neuron_lif_voltage_update(Neuron* neuron, f32 psc, u32 time) {
    _NEURON_LIF_VOLTAGE_UPDATE(neuron, psc);
}

internal void
_neuron_lif_refract_voltage_update(Neuron* neuron, f32 psc, u32 time) {
    // NOTE: Only update after refractory period
    if (time - neuron->last_spike_time <= neuron->cls->lif_refract_cls.refract_time) {
        neuron->voltage = NEURON_LIF_VOLTAGE_REST;
    } else {
        _NEURON_LIF_VOLTAGE_UPDATE(neuron, psc);
    }
}

typedef void (*NEURON_UPDATE_FN)(Neuron* neuron, f32 psc, u32 time);
NEURON_UPDATE_FN _NEURON_UPDATE_FNS[NEURON_TYPES_CNT] = {
    _neuron_if_voltage_update,
    _neuron_if_refract_voltage_update,
    _neuron_if_one_spike_voltage_update,
    _neuron_lif_voltage_update,
    _neuron_lif_refract_voltage_update,
};

internal void
_neuron_update(Neuron* neuron, u32 time, f32 psc) {
    NeuronType n_type = neuron->cls->type;

    _NEURON_UPDATE_FNS[n_type](neuron, psc, time);
    if (neuron->voltage >= NEURON_VOLTAGE_TH[n_type]) {
        neuron->voltage = NEURON_VOLTAGE_REST[n_type];
        neuron->spike = TRUE;
        neuron->last_spike_time = time;
    } else {
        neuron->spike = FALSE;
    }
}


internal void
_neuron_update_in_synapses(Neuron* neuron, u32 time) {
    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    SynapseRefArray* it = NULL;

    for (it = neuron->in_synapse_arrays; it != NULL; it = it->next) {
        for (synapse_i = 0; synapse_i < it->length; ++synapse_i) {
            synapse = it->synapses[synapse_i];
            synapse_step(synapse, time);
        }
    }
}


internal void
_neuron_update_out_synapses(Neuron* neuron, u32 time) {
    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    SynapseRefArray* it = NULL;

    if (neuron->spike == FALSE) return;

    for (it = neuron->out_synapse_arrays; it != NULL; it = it->next) {
        for (synapse_i = 0; synapse_i < it->length; ++synapse_i) {
            synapse = it->synapses[synapse_i];
            synapse_add_spike_time(synapse, time);
        }
    }
}


internal void
neuron_step(Neuron* neuron, u32 time) {
    TIMING_COUNTER_START(NEURON_STEP);

    check(neuron != NULL, "neuron is NULL");

    f32 psc = _neuron_compute_psc(neuron, time);
    _neuron_update(neuron, time, psc);
    _neuron_update_out_synapses(neuron, time);

    TIMING_COUNTER_END(NEURON_STEP);

    error:
    return;
}


internal void
neuron_step_force_spike(Neuron* neuron, u32 time) {
    TIMING_COUNTER_START(NEURON_STEP_FORCE_SPIKE);

    check(neuron != NULL, "neuron is NULL");

    _neuron_update_in_synapses(neuron, time);
    neuron->spike = TRUE;
    neuron->last_spike_time = time;
    neuron->voltage = NEURON_VOLTAGE_REST[neuron->cls->type];
    _neuron_update_out_synapses(neuron, time);

    TIMING_COUNTER_END(NEURON_STEP_FORCE_SPIKE);

    error:
    return;
}


internal void
neuron_step_inject_current(Neuron* neuron, f32 psc, u32 time) {
    TIMING_COUNTER_START(NEURON_STEP_INJECT_CURRENT);

    check(neuron != NULL, "neuron is NULL");

    psc = _neuron_compute_psc(neuron, time) + psc;
    _neuron_update(neuron, time, psc);
    _neuron_update_out_synapses(neuron, time);

    TIMING_COUNTER_END(NEURON_STEP_INJECT_CURRENT);

    error:
    return;
}


internal void
neuron_clear(Neuron* neuron) {
    check(neuron != NULL, "neuron is NULL");

    neuron->spike = FALSE;
    neuron->last_spike_time = INVALID_SPIKE_TIME;
    neuron->voltage = NEURON_VOLTAGE_REST[neuron->cls->type];

    u32 synapse_i = 0;
    Synapse* synapse = NULL;
    SynapseRefArray* it = NULL;

    for (it = neuron->in_synapse_arrays; it != NULL; it = it->next) {
        for (synapse_i = 0; synapse_i < it->length; ++synapse_i) {
            synapse = it->synapses[synapse_i];
            synapse_clear(synapse);
        }
    }

    error:
    return;
}