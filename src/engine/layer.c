/******************
*   LAYER TYPE
******************/
internal char*
layer_type_get_c_str(LayerType type) {
    if (type == LAYER_DENSE) return "LAYER_DENSE";
    else return "LAYER_INVALID";
}


/****************
*   LAYER
****************/
internal Layer*
layer_create(State* state, const char* name, LayerType type, u32 n_neurons, NeuronCls* cls) {
    Layer* layer = NULL;
    u32 i = 0;

    check(state != NULL, "state is NULL");
    check(name != NULL, "name is NULL");
    check(type == LAYER_DENSE, "invalid layer type %s",
          layer_type_get_c_str(type));
    check(n_neurons > 0, "n_neurons is 0");
    check(cls != NULL, "cls is NULL");

    layer = (Layer*) memory_push(state->permanent_storage, sizeof(*layer));
    check_memory(layer);

    layer->type = type;
    layer->name = string_create(state->permanent_storage, name);
    check_memory(layer->name);
    layer->neuron_cls = cls;
    layer->inputs = NULL;
    layer->outputs = NULL;
    layer->n_inputs = 0;
    layer->n_outputs = 0;

    layer->n_neurons = n_neurons;
    layer->neuron_start_i = 0;
    layer->neuron_end_i = 0;

    layer->it_ran = FALSE;

    return layer;
    error:
    return NULL;
}


internal void
layer_process_neurons(void* task) {
    LayerTask* layer_task = (LayerTask*)task;

    // NOTE: UNWRAP the task structure
    u32 time = layer_task->time;
    Layer* layer = layer_task->layer;
    Neuron* neurons = layer_task->neurons + layer->neuron_start_i;
    Synapse* synapses = layer_task->synapses;
    u32 neuron_start_i = layer_task->neuron_start_i;
    u32 neuron_end_i = layer_task->neuron_end_i;

    // NOTE: here we will use only one set of variables to remove a lot of duplicate code
    f32* currents = NULL;
    u32 n_currents = 0;
    u32 currents_idx = 0;
    if (layer_task->data->currents) {
        currents = layer_task->data->currents->currents;
        n_currents = layer_task->data->currents->n_currents;
        currents_idx = math_clip_u32(n_currents, neuron_start_i, neuron_end_i);
    }

    bool* spikes = NULL;
    u32 n_spikes = 0;
    u32 spikes_idx = 0;
    if (layer_task->data->spikes) {
        spikes = layer_task->data->spikes->spikes;
        n_spikes = layer_task->data->spikes->n_spikes;
        spikes_idx = math_clip_u32(n_spikes, neuron_start_i, neuron_end_i);
    }

    u32 i = 0;
    LayerTaskType task_type = layer_task->data->type;
    if (task_type == LAYER_TASK_STEP) {
        for (i = neuron_start_i; i < neuron_end_i; ++i)
            neuron_step(neurons + i, synapses, time);
    }
    else if (task_type == LAYER_TASK_STEP_INJECT_CURRENT) {
        for (i = neuron_start_i; i < currents_idx; ++i)
            neuron_step_inject_current(neurons + i, synapses, currents[i], time);
        for (i = currents_idx; i < neuron_end_i; ++i)
            neuron_step(neurons + i, synapses, time);
    }
    else if (task_type == LAYER_TASK_STEP_FORCE_SPIKE) {
        for (i = neuron_start_i; i < spikes_idx; ++i) {
            if (spikes[i] == TRUE)
                neuron_step_force_spike(neurons + i, synapses, time);
            else
                neuron_step(neurons + i, synapses, time);
        }
        for (i = spikes_idx; i < neuron_end_i; ++i)
            neuron_step(neurons + i, synapses, time);
    }
    // LEARNING
    else if (task_type == LAYER_TASK_LEARNING_STEP) {
        for (i = neuron_start_i; i < neuron_end_i; ++i)
            neuron_learning_step(neurons + i, neurons, synapses, time);
    }
    else if (task_type == LAYER_TASK_LEARNING_STEP_INJECT_CURRENT) {
        for (i = neuron_start_i; i < currents_idx; ++i)
            neuron_learning_step_inject_current(neurons + i, neurons, synapses, currents[i], time);
        for (i = currents_idx; i < neuron_end_i; ++i)
            neuron_learning_step(neurons + i, neurons, synapses, time);
    }
    else if (task_type == LAYER_TASK_LEARNING_STEP_FORCE_SPIKE) {
        for (i = neuron_start_i; i < spikes_idx; ++i) {
            if (spikes[i] == TRUE)
                neuron_learning_step_force_spike(neurons + i, neurons, synapses, time);
            else
                neuron_learning_step(neurons + i, neurons, synapses, time);
        }
        for (i = spikes_idx; i < neuron_end_i; ++i)
            neuron_learning_step(neurons + i, neurons, synapses, time);
    }
    else {
        log_error("Unknown LAYER_TASK_TYPE: %u", layer_task->data->type);
    }
}


internal LayerTask*
_layer_task_create(Layer* layer, Neuron* neurons, Synapse* synapses, Memory* memory, u32 time,
                   u32 task_i, u32 n_neurons_per_task, LayerStepData* data) {
    LayerTask* task = (LayerTask*) memory_push(memory, sizeof(*task));
    check(task != NULL, "task is NULL");

    task->data = data;
    task->layer = layer;
    task->neurons = neurons;
    task->synapses = synapses;
    task->time = time;
    task->neuron_start_i = task_i * n_neurons_per_task;
    task->neuron_end_i = task->neuron_start_i + n_neurons_per_task;
    if (task->neuron_end_i > layer->n_neurons)
        task->neuron_end_i = layer->n_neurons;

    error:
    return task;
}


internal u32
_layer_get_n_tasks(ThreadPool* pool) {
    u32 result = pool->n_threads + 1; // consider also the main thread
    return result;
}


internal u32
_layer_get_n_neurons_per_task(Layer* layer, u32 n_tasks) {
    u32 result = (layer->n_neurons + n_tasks - 1) / n_tasks;
    return result;
}


internal void
_layer_run(Layer* layer, Neuron* neurons, Synapse* synapses, u32 time, Memory* memory, ThreadPool* pool, LayerStepData* data) {
    check(layer != NULL, "layer is NULL");
    check(neurons != NULL, "neurons is NULL");
    check(synapses != NULL, "synapses is NULL");
    check(memory != NULL, "memory is NULL");
    check(pool != NULL, "pool is NULL");
    check(data != NULL, "data is NULL");

    thread_pool_reset(pool);

    u32 n_tasks = _layer_get_n_tasks(pool);
    u32 n_neurons_per_task = _layer_get_n_neurons_per_task(layer, n_tasks);
    LayerTask* task = NULL;

    for (u32 task_i = 0; task_i < n_tasks; ++task_i) {
        task = _layer_task_create(layer, neurons, synapses, memory, time, task_i, n_neurons_per_task, data);
        thread_pool_add_task(pool, task);
    }

    thread_pool_execute_tasks(pool);

    error:
    return;
}


internal void
layer_step(Layer* layer, Neuron* neurons, Synapse* synapses, u32 time, Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(LAYER_STEP);
    // TODO: Why not checks?
    LayerStepData data = { 0 };
    data.type = LAYER_TASK_STEP;

    _layer_run(layer, neurons, synapses, time, memory, pool, &data);

    TIMING_COUNTER_END(LAYER_STEP);
}


internal void
layer_step_inject_current(Layer* layer, Neuron* neurons, Synapse* synapses, u32 time, Currents* currents,
                          Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(LAYER_STEP_INJECT_CURRENT);
    // TODO: Why not checks?

    LayerStepData data;
    data.type = LAYER_TASK_STEP_INJECT_CURRENT;
    data.currents = currents;

    _layer_run(layer, neurons, synapses, time, memory, pool, &data);

    TIMING_COUNTER_END(LAYER_STEP_INJECT_CURRENT);
}


internal void
layer_step_force_spike(Layer* layer, Neuron* neurons, Synapse* synapses, u32 time, Spikes* spikes,
                       Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(LAYER_STEP_FORCE_SPIKE);
    // TODO: Why not checks?

    LayerStepData data;
    data.type = LAYER_TASK_STEP_FORCE_SPIKE;
    data.spikes = spikes;

    _layer_run(layer, neurons, synapses, time, memory, pool, &data);

    TIMING_COUNTER_END(LAYER_STEP_FORCE_SPIKE);
}


internal void
layer_clear(Layer* layer, Neuron* neurons, Synapse* synapses) {
    check(layer != NULL, "layer is NULL");

    for (u32 i = layer->neuron_start_i; i < layer->neuron_end_i; ++i)
        neuron_clear(neurons + i, synapses);

    layer->it_ran = FALSE;

    error:
    return;
}


internal u32
layer_get_n_in_synapses(Layer* layer, Neuron* neurons) {
    check(layer != NULL, "layer is NULL");
    check(neurons != NULL, "neurons is NULL");

    u32 n_in_synapses = 0;
    u32 i = 0;
    u32 j = 0;
    Neuron* neuron = NULL;
    SynapseIdxArray* it = NULL;

    for (i = layer->neuron_start_i; i < layer->neuron_end_i; ++i) {
        neuron = neurons + i;

        for (it = neuron->in_synapse_arrays; it != NULL; it = it->next) {
            n_in_synapses += it->length;
        }
    }

    return n_in_synapses;
    error:
    return 0;
}


internal void
layer_show(Layer* layer, Neuron* neurons) {
    check(layer != NULL, "layer is NULL");

    u32 i = 0;
    u32 n_in_synapses = layer_get_n_in_synapses(layer, neurons);
    LayerLink* link = NULL;

    printf("-------------------\n");
    printf("Name: %s\n", string_get_c_str(layer->name));
    printf("Type: %s\n", layer_type_get_c_str(layer->type));
    printf("Number of neurons %u of type %s\n",
           layer->n_neurons,
           string_get_c_str(layer->neuron_cls->name));
    printf("Number of input synapses %u\n", n_in_synapses);

    printf("Input layers: ");
    for (link = layer->inputs; link != NULL; link = link->next) {
        printf("%s, ", string_get_c_str(link->layer->name));
    }

    printf("\nOutput layers: ");
    for (link = layer->outputs; link != NULL; link = link->next) {
        printf("%s, ", string_get_c_str(link->layer->name));
    }

    printf("\n-------------------\n\n");
    error:
    return;
}


internal bool
_layer_link_dense(State* state, Layer* layer, LayerLink* link, Neuron* neurons, Synapse* synapses, u32 offset) {
    u32 neuron_i = 0;
    Neuron* neuron = NULL;
    Layer* in_layer = link->layer;
    SynapseIdxArray* synapse_idxs = NULL;

    f32 chance = link->chance;
    if (chance < 1.0f) chance = math_clip_f32(chance + 0.1f, 0.0f, 1.0f);
    u32 n_synapses_per_neuron = (u32)(chance * in_layer->n_neurons);

    for (neuron_i = layer->neuron_start_i; neuron_i < layer->neuron_end_i; ++neuron_i) {
        synapse_idxs = memory_push(state->permanent_storage,
                            sizeof(*synapse_idxs) * n_synapses_per_neuron * sizeof(u32));
        check_memory(synapse_idxs);
        synapse_idxs->capacity = n_synapses_per_neuron;
        synapse_idxs->length = 0;
        synapse_idxs->idxs = (u32*)(synapse_idxs + 1);

        neuron = &neurons[neuron_i];
        synapse_idxs->next = neuron->in_synapse_arrays != NULL ? neuron->in_synapse_arrays : NULL;
        neuron->in_synapse_arrays = synapse_idxs;
    }

    // TODO: <= or just <??
    n_synapses_per_neuron = (u32)(chance * layer->n_neurons);
    for (neuron_i = in_layer->neuron_start_i; neuron_i < in_layer->neuron_end_i; ++neuron_i) {
        synapse_idxs = memory_push(state->permanent_storage,
                            sizeof(*synapse_idxs) * n_synapses_per_neuron * sizeof(u32));
        check_memory(synapse_idxs);
        synapse_idxs->capacity = n_synapses_per_neuron;
        synapse_idxs->length = 0;
        synapse_idxs->idxs = (u32*)(synapse_idxs + 1);
        // TODO: after it works make a create synapse_idxs

        Neuron* neuron = &neurons[neuron_i];
        synapse_idxs->next = neuron->out_synapse_arrays != NULL ? neuron->out_synapse_arrays : NULL;
        neuron->out_synapse_arrays = synapse_idxs;
    }

    synapse_idxs = NULL;
    u32 out_neuron_i = 0;
    u32 in_neuron_i = 0;
    Neuron* in_neuron = NULL;
    Neuron* out_neuron = NULL;
    Synapse* synapse = NULL;
    for (out_neuron_i = layer->neuron_start_i; out_neuron_i < layer->neuron_end_i; ++out_neuron_i) {
        for (in_neuron_i = in_layer->neuron_start_i; in_neuron_i < in_layer->neuron_end_i; ++in_neuron_i) {
            if (random_get_chance_f32() > link->chance) continue;

            synapse = &synapses[offset];
            synapse_init(synapse, link->cls, link->weight);

            in_neuron = &neurons[in_neuron_i];
            synapse_idxs = in_neuron->out_synapse_arrays;
            synapse_idxs->idxs[synapse_idxs->length++] = offset;

            out_neuron = &neurons[out_neuron_i];
            synapse_idxs = out_neuron->in_synapse_arrays;
            synapse_idxs->idxs[synapse_idxs->length++] = offset;

            ++offset;
        }
    }

    error:
    return offset;
}


internal u32
layer_link_synapses(State* state, Layer* layer, LayerLink* link, Neuron* neurons, Synapse* synapses, u32 offset) {
    bool status = FALSE;
    check(state != NULL, "state is NULL");
    check(layer != NULL, "layer is NULL");
    check(link != NULL, "link is NULL");
    check(neurons != NULL, "neurons is NULL");
    check(synapses != NULL, "synapses is NULL");
    check(offset <= (u32)-1, "offset is too big %u", offset);

    if (layer->type == LAYER_DENSE) {
        offset = _layer_link_dense(state, layer, link, neurons, synapses, offset);
    } else {
        log_error("Unknown layer type %u", layer->type);
    }

    error:
    return offset;
}


internal void
layer_init_neurons(Layer* layer, Neuron* neurons) {
    check(layer != NULL, "layer is NULL");
    check(neurons != NULL, "neurons is NULL");

    u32 neuron_i = 0;
    Neuron* neuron = NULL;
    for (neuron_i = layer->neuron_start_i; neuron_i < layer->neuron_end_i; ++neuron_i) {
        neuron = neurons + neuron_i;
        neuron_init(neuron, layer->neuron_cls);
    }

    error:
    return;
}

internal bool
layer_link(State* state, Layer* layer, Layer* in_layer, SynapseCls* cls, f32 weight, f32 chance) {
    bool status = FALSE;
    check(state != NULL, "state is NULL");
    check(layer != NULL, "layer is NULL");
    check(in_layer != NULL, "input_layer is NULL");
    check(cls != NULL, "cls is NULL");
    check(chance >= 0.0f && chance <= 1.0f, "chance should be in [0, 1]");

    // NOTE: Save references between layers
    LayerLink* link = (LayerLink*) memory_push(state->permanent_storage, sizeof(*link));
    check_memory(link);
    link->layer = in_layer;
    link->cls = cls;
    link->weight = weight;
    link->chance = chance;

    if (layer->inputs == NULL) {
        link->next = NULL;
        layer->inputs = link;
    } else {
        link->next = layer->inputs;
        layer->inputs = link;
    }
    layer->n_inputs++;

    link = (LayerLink*) memory_push(state->permanent_storage, sizeof(*link));
    check_memory(link);
    link->layer = layer;
    link->cls = cls;
    link->weight = weight;
    link->chance = chance;

    if (in_layer->outputs == NULL) {
        link->next = NULL;
        in_layer->outputs = link;
    } else {
        link->next = in_layer->outputs;
        in_layer->outputs = link;
    }
    in_layer->n_outputs++;

    error:
    return status;
}


internal f32*
layer_get_voltages(Memory* memory, Layer* layer, Neuron* neurons) {
    check(memory != NULL, "memory is NULL");
    check(layer != NULL, "layer is NULL");
    check(neurons != NULL, "neurons is NULL");

    f32* voltages = (f32*)memory_push(memory, sizeof(f32) * layer->n_neurons);
    check_memory(voltages);

    for (u32 i = layer->neuron_start_i; i < layer->neuron_end_i; ++i)
        voltages[i] = (neurons + i)->voltage;

    return voltages;

    error:
    return NULL;
}


internal f32*
layer_get_pscs(Memory* memory, Layer* layer, Neuron* neurons) {
    check(memory != NULL, "memory is NULL");
    check(layer != NULL, "layer is NULL");
    check(neurons != NULL, "neurons is NULL");

    f32* pscs = (f32*)memory_push(memory, sizeof(f32) * layer->n_neurons);
    check_memory(pscs);

    for (u32 i = 0; i < layer->n_neurons; ++i)
        pscs[i] = (neurons + i)->epsc + (neurons + i)->ipsc;

    return pscs;

    error:
    return NULL;
}


internal f32*
layer_get_epscs(Memory* memory, Layer* layer, Neuron* neurons) {
    check(memory != NULL, "memory is NULL");
    check(layer != NULL, "layer is NULL");
    check(neurons != NULL, "neurons is NULL");

    f32* epscs = (f32*)memory_push(memory, sizeof(f32) * layer->n_neurons);
    check_memory(epscs);

    for (u32 i = 0; i < layer->n_neurons; ++i)
        epscs[i] = (neurons + i)->epsc;

    return epscs;

    error:
    return NULL;
}


internal f32*
layer_get_ipscs(Memory* memory, Layer* layer, Neuron* neurons) {
    check(memory != NULL, "memory is NULL");
    check(layer != NULL, "layer is NULL");
    check(neurons != NULL, "neurons is NULL");

    f32* ipscs = (f32*)memory_push(memory, sizeof(f32) * layer->n_neurons);
    check_memory(ipscs);

    for (u32 i = 0; i < layer->n_neurons; ++i)
        ipscs[i] = (neurons + i)->ipsc;

    return ipscs;

    error:
    return NULL;
}


internal bool*
layer_get_spikes(Memory* memory, Layer* layer, Neuron* neurons) {
    check(memory != NULL, "memory is NULL");
    check(layer != NULL, "layer is NULL");
    check(neurons != NULL, "neurons is NULL");

    bool* spikes = (bool*)memory_push(memory, sizeof(bool) * layer->n_neurons);
    check_memory(spikes);

    for (u32 i = 0; i < layer->n_neurons; ++i)
        spikes[i] = (neurons + i)->spike;

    return spikes;

    error:
    return NULL;
}


internal void
layer_learning_step(Layer* layer, Neuron* neurons, Synapse* synapses, u32 time, Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(LAYER_LEARNING_STEP);

    LayerStepData data = { 0 };
    data.type = LAYER_TASK_LEARNING_STEP;

    _layer_run(layer, neurons, synapses, time, memory, pool, &data);

    TIMING_COUNTER_END(LAYER_LEARNING_STEP);
}


internal void
layer_learning_step_inject_current(Layer* layer, Neuron* neurons, Synapse* synapses, u32 time, Currents* currents,
                                   Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(LAYER_LEARNING_STEP_INJECT_CURRENT);

    LayerStepData data;
    data.type = LAYER_TASK_LEARNING_STEP_INJECT_CURRENT;
    data.currents = currents;

    _layer_run(layer, neurons, synapses, time, memory, pool, &data);

    TIMING_COUNTER_END(LAYER_LEARNING_STEP_INJECT_CURRENT);
}


internal void
layer_learning_step_force_spike(Layer* layer, Neuron* neurons, Synapse* synapses, u32 time, Spikes* spikes,
                                Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(LAYER_LEARNING_STEP_FORCE_SPIKE);

    LayerStepData data;
    data.type = LAYER_TASK_LEARNING_STEP_FORCE_SPIKE;
    data.spikes = spikes;

    _layer_run(layer, neurons, synapses, time, memory, pool, &data);

    TIMING_COUNTER_END(LAYER_LEARNING_STEP_FORCE_SPIKE);
}
