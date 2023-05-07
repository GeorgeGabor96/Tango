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
layer_create(Memory* memory, String* name, LayerType type, u32 n_neurons, NeuronCls* cls) {
    check(memory != NULL, "memory is NULL");
    check(name != NULL, "name is NULL");
    check(type == LAYER_DENSE, "invalid layer type %s",
          layer_type_get_c_str(type));
    check(n_neurons > 0, "n_neurons is 0");
    check(cls != NULL, "cls is NULL");

    Layer* layer = (Layer*) memory_push(memory, sizeof(*layer));
    check_memory(layer);

    layer->type = type;
    layer->name = name;
    layer->neuron_cls = cls;
    layer->inputs = NULL;
    layer->outputs = NULL;
    layer->n_inputs = 0;
    layer->n_outputs = 0;

    layer->neurons = NULL;
    layer->n_neurons = n_neurons;

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
    Neuron* neurons = layer->neurons;
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

    b32* spikes = NULL;
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
            neuron_step(neurons + i, time);
    }
    else if (task_type == LAYER_TASK_STEP_INJECT_CURRENT) {
        for (i = neuron_start_i; i < currents_idx; ++i)
            neuron_step_inject_current(neurons + i, currents[i], time);
        for (i = currents_idx; i < neuron_end_i; ++i)
            neuron_step(neurons + i, time);
    }
    else if (task_type == LAYER_TASK_STEP_FORCE_SPIKE) {
        for (i = neuron_start_i; i < spikes_idx; ++i) {
            if (spikes[i] == TRUE)
                neuron_step_force_spike(neurons + i, time);
            else
                neuron_step(neurons + i, time);
        }
        for (i = spikes_idx; i < neuron_end_i; ++i)
            neuron_step(neurons + i, time);
    }
    // LEARNING
    else if (task_type == LAYER_TASK_LEARNING_STEP) {
        for (i = neuron_start_i; i < neuron_end_i; ++i)
            neuron_learning_step(neurons + i, time);
    }
    else if (task_type == LAYER_TASK_LEARNING_STEP_INJECT_CURRENT) {
        for (i = neuron_start_i; i < currents_idx; ++i)
            neuron_learning_step_inject_current(neurons + i, currents[i], time);
        for (i = currents_idx; i < neuron_end_i; ++i)
            neuron_learning_step(neurons + i, time);
    }
    else if (task_type == LAYER_TASK_LEARNING_STEP_FORCE_SPIKE) {
        for (i = neuron_start_i; i < spikes_idx; ++i) {
            if (spikes[i] == TRUE)
                neuron_learning_step_force_spike(neurons + i, time);
            else
                neuron_learning_step(neurons + i, time);
        }
        for (i = spikes_idx; i < neuron_end_i; ++i)
            neuron_learning_step(neurons + i, time);
    }
    else {
        log_error("Unknown LAYER_TASK_TYPE: %u", layer_task->data->type);
    }
}


internal LayerTask*
_layer_task_create(Layer* layer, Memory* memory, u32 time,
                   u32 task_i, u32 n_neurons_per_task, LayerStepData* data) {
    LayerTask* task = (LayerTask*) memory_push(memory, sizeof(*task));
    check(task != NULL, "task is NULL");

    task->data = data;
    task->layer = layer;
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
_layer_run(Layer* layer, u32 time, Memory* memory, ThreadPool* pool, LayerStepData* data) {
    check(layer != NULL, "layer is NULL");
    check(memory != NULL, "memory is NULL");
    check(pool != NULL, "pool is NULL");
    check(data != NULL, "data is NULL");

    thread_pool_reset(pool);

    u32 n_tasks = _layer_get_n_tasks(pool);
    u32 n_neurons_per_task = _layer_get_n_neurons_per_task(layer, n_tasks);
    LayerTask* task = NULL;

    for (u32 task_i = 0; task_i < n_tasks; ++task_i) {
        task = _layer_task_create(layer, memory, time, task_i, n_neurons_per_task, data);
        thread_pool_add_task(pool, task);
    }

    thread_pool_execute_tasks(pool);

    error:
    return;
}


internal void
layer_step(Layer* layer, u32 time, Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(LAYER_STEP);
    LayerStepData data = { 0 };
    data.type = LAYER_TASK_STEP;

    _layer_run(layer, time, memory, pool, &data);

    TIMING_COUNTER_END(LAYER_STEP);
}


internal void
layer_step_inject_current(Layer* layer, u32 time, Currents* currents,
                          Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(LAYER_STEP_INJECT_CURRENT);

    LayerStepData data;
    data.type = LAYER_TASK_STEP_INJECT_CURRENT;
    data.currents = currents;

    _layer_run(layer, time, memory, pool, &data);

    TIMING_COUNTER_END(LAYER_STEP_INJECT_CURRENT);
}


internal void
layer_step_force_spike(Layer* layer, u32 time, Spikes* spikes,
                       Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(LAYER_STEP_FORCE_SPIKE);

    LayerStepData data;
    data.type = LAYER_TASK_STEP_FORCE_SPIKE;
    data.spikes = spikes;

    _layer_run(layer, time, memory, pool, &data);

    TIMING_COUNTER_END(LAYER_STEP_FORCE_SPIKE);
}


internal void
layer_clear(Layer* layer) {
    check(layer != NULL, "layer is NULL");
    Neuron* neurons = layer->neurons;

    for (u32 i = 0; i < layer->n_neurons; ++i)
        neuron_clear(neurons + i);

    layer->it_ran = FALSE;

    error:
    return;
}


internal u32
layer_get_n_in_synapses(Layer* layer) {
    check(layer != NULL, "layer is NULL");

    u32 n_in_synapses = 0;
    u32 i = 0;
    u32 j = 0;
    Neuron* neuron = NULL;
    Neuron* neurons = layer->neurons;
    SynapseRefArray* it = NULL;

    for (i = 0; i < layer->n_neurons; ++i) {
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
layer_show(Layer* layer) {
    check(layer != NULL, "layer is NULL");

    u32 i = 0;
    u32 n_in_synapses = layer_get_n_in_synapses(layer);
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


internal b32
_layer_link_dense(Layer* layer, LayerLink* link, Synapse* synapses, u32 offset, Memory* memory, Random* random) {
    u32 neuron_i = 0;
    Neuron* neuron = NULL;
    Layer* in_layer = link->layer;
    SynapseRefArray* synapse_refs = NULL;

    // NOTE: Consider 10% more synapses just so we could allocate enough in a continuous array
    f32 synapse_chance = link->synapse_chance;
    if (synapse_chance < 1.0f) synapse_chance = math_clip_f32(synapse_chance + 0.1f, 0.0f, 1.0f);
    u32 n_synapses_per_neuron = (u32)(synapse_chance * in_layer->n_neurons);

    for (neuron_i = 0; neuron_i < layer->n_neurons; ++neuron_i) {
        synapse_refs = neuron_create_synapses_ref_array(memory, n_synapses_per_neuron);
        check_memory(synapse_refs);

        neuron = layer->neurons + neuron_i;
        synapse_refs->next = neuron->in_synapse_arrays != NULL ? neuron->in_synapse_arrays : NULL;
        neuron->in_synapse_arrays = synapse_refs;
    }

    n_synapses_per_neuron = (u32)(synapse_chance * layer->n_neurons);
    for (neuron_i = 0; neuron_i < in_layer->n_neurons; ++neuron_i) {
        synapse_refs = neuron_create_synapses_ref_array(memory, n_synapses_per_neuron);
        check_memory(synapse_refs);

        neuron = in_layer->neurons + neuron_i;
        synapse_refs->next = neuron->out_synapse_arrays != NULL ? neuron->out_synapse_arrays : NULL;
        neuron->out_synapse_arrays = synapse_refs;
    }

    synapse_refs = NULL;
    u32 out_neuron_i = 0;
    u32 in_neuron_i = 0;
    f32 weight = 0.0f;
    Neuron* in_neuron = NULL;
    Neuron* out_neuron = NULL;
    Synapse* synapse = NULL;
    for (out_neuron_i = 0; out_neuron_i < layer->n_neurons; ++out_neuron_i) {
        for (in_neuron_i = 0; in_neuron_i < in_layer->n_neurons; ++in_neuron_i) {
            if (random_get_chance_f32(random) > link->synapse_chance) continue;

            synapse = synapses + offset;

            // NOTE: weight of 0 means dead synapse, don't add them
            weight = 0;
            while (math_float_equals_f32(weight, 0.0f)) {
                weight = random_get_in_range_f32(random, link->min_weight, link->max_weight);
            }
            synapse_init(synapse, link->cls, weight);

            in_neuron = in_layer->neurons + in_neuron_i;
            synapse_refs = in_neuron->out_synapse_arrays;
            synapse_refs->synapses[synapse_refs->length++] = synapse;

            out_neuron = layer->neurons + out_neuron_i;
            synapse_refs = out_neuron->in_synapse_arrays;
            synapse_refs->synapses[synapse_refs->length++] = synapse;

            synapse->in_neuron = in_neuron;
            synapse->out_neuron = out_neuron;

            ++offset;
        }
    }

    error:
    return offset;
}


internal u32
layer_link_synapses(Layer* layer, LayerLink* link, Synapse* synapses, u32 offset, Memory* memory, Random* random) {
    check(layer != NULL, "layer is NULL");
    check(link != NULL, "link is NULL");
    check(synapses != NULL, "synapses is NULL");
    check(offset <= (u32)-1, "offset is too big %u", offset);
    check(memory != NULL, "memory is NULL");
    check(random != NULL, "random is NULL");

    if (layer->type == LAYER_DENSE) {
        offset = _layer_link_dense(layer, link, synapses, offset, memory, random);
    } else {
        log_error("Unknown layer type %u", layer->type);
    }

    error:
    return offset;
}


internal void
layer_init_neurons(Layer* layer) {
    check(layer != NULL, "layer is NULL");

    u32 neuron_i = 0;
    Neuron* neurons = layer->neurons;
    for (neuron_i = 0; neuron_i < layer->n_neurons; ++neuron_i) {
        neuron_init(neurons + neuron_i, layer->neuron_cls);
    }

    error:
    return;
}


LayerLink* _layer_link_create(Layer* layer, SynapseCls* cls,
                              f32 min_weight, f32 max_weight, f32 synapse_chance,
                              Memory* memory) {
    LayerLink* link = (LayerLink*) memory_push(memory, sizeof(*link));
    check_memory(link);
    link->layer = layer;
    link->cls = cls;
    link->min_weight = min_weight;
    link->max_weight = max_weight;
    link->synapse_chance = synapse_chance;

    error:
    return link;
}


LayerLink* _layer_link_add(LayerLink* chain, LayerLink* link) {
    link->next = chain != NULL ? chain : NULL;
    return link;
}


internal b32
layer_link(Layer* layer, Layer* in_layer, SynapseCls* cls,
           f32 min_weight, f32 max_weight, f32 synapse_chance, Memory* memory) {
    b32 status = FALSE;
    check(layer != NULL, "layer is NULL");
    check(in_layer != NULL, "input_layer is NULL");
    check(cls != NULL, "cls is NULL");
    check(min_weight >= 0.0f, "min_weight %f should be positive", min_weight);
    check(max_weight >= 0.0f, "max_weight %f should be positive", max_weight);
    check(min_weight <= max_weight, "min_weight %f > max_weight %f", min_weight, max_weight);
    check(synapse_chance >= 0.0f && synapse_chance <= 1.0f, "synapse_chance should be in [0, 1]");
    check(memory != NULL, "memory is NULL");

    // NOTE: Save references between layers
    LayerLink* link = _layer_link_create(in_layer, cls, min_weight, max_weight, synapse_chance, memory);
    check_memory(link);
    layer->inputs = _layer_link_add(layer->inputs, link);
    layer->n_inputs++;

    link = _layer_link_create(layer, cls, min_weight, max_weight, synapse_chance, memory);
    check_memory(link);
    in_layer->outputs = _layer_link_add(layer->outputs, link);
    in_layer->n_outputs++;

    error:
    return status;
}


internal f32*
layer_get_voltages(Memory* memory, Layer* layer) {
    check(memory != NULL, "memory is NULL");
    check(layer != NULL, "layer is NULL");

    f32* voltages = (f32*)memory_push(memory, sizeof(f32) * layer->n_neurons);
    check_memory(voltages);
    Neuron* neurons = layer->neurons;

    for (u32 i = 0; i < layer->n_neurons; ++i)
        voltages[i] = (neurons + i)->voltage;

    return voltages;

    error:
    return NULL;
}


internal f32*
layer_get_pscs(Memory* memory, Layer* layer) {
    check(memory != NULL, "memory is NULL");
    check(layer != NULL, "layer is NULL");

    f32* pscs = (f32*)memory_push(memory, sizeof(f32) * layer->n_neurons);
    check_memory(pscs);
    Neuron* neurons = layer->neurons;

    for (u32 i = 0; i < layer->n_neurons; ++i)
        pscs[i] = (neurons + i)->epsc + (neurons + i)->ipsc;

    return pscs;

    error:
    return NULL;
}


internal f32*
layer_get_epscs(Memory* memory, Layer* layer) {
    check(memory != NULL, "memory is NULL");
    check(layer != NULL, "layer is NULL");

    f32* epscs = (f32*)memory_push(memory, sizeof(f32) * layer->n_neurons);
    check_memory(epscs);
    Neuron* neurons = layer->neurons;

    for (u32 i = 0; i < layer->n_neurons; ++i)
        epscs[i] = (neurons + i)->epsc;

    return epscs;

    error:
    return NULL;
}


internal f32*
layer_get_ipscs(Memory* memory, Layer* layer) {
    check(memory != NULL, "memory is NULL");
    check(layer != NULL, "layer is NULL");

    f32* ipscs = (f32*)memory_push(memory, sizeof(f32) * layer->n_neurons);
    check_memory(ipscs);
    Neuron* neurons = layer->neurons;

    for (u32 i = 0; i < layer->n_neurons; ++i)
        ipscs[i] = (neurons + i)->ipsc;

    return ipscs;

    error:
    return NULL;
}


internal b32*
layer_get_spikes(Memory* memory, Layer* layer) {
    check(memory != NULL, "memory is NULL");
    check(layer != NULL, "layer is NULL");

    b32* spikes = (b32*)memory_push(memory, sizeof(b32) * layer->n_neurons);
    check_memory(spikes);
    Neuron* neurons = layer->neurons;

    for (u32 i = 0; i < layer->n_neurons; ++i)
        spikes[i] = (neurons + i)->spike;

    return spikes;

    error:
    return NULL;
}


internal void
layer_learning_step(Layer* layer, u32 time, Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(LAYER_LEARNING_STEP);

    LayerStepData data = { 0 };
    data.type = LAYER_TASK_LEARNING_STEP;

    _layer_run(layer, time, memory, pool, &data);

    TIMING_COUNTER_END(LAYER_LEARNING_STEP);
}


internal void
layer_learning_step_inject_current(Layer* layer, u32 time, Currents* currents,
                                   Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(LAYER_LEARNING_STEP_INJECT_CURRENT);

    LayerStepData data;
    data.type = LAYER_TASK_LEARNING_STEP_INJECT_CURRENT;
    data.currents = currents;

    _layer_run(layer, time, memory, pool, &data);

    TIMING_COUNTER_END(LAYER_LEARNING_STEP_INJECT_CURRENT);
}


internal void
layer_learning_step_force_spike(Layer* layer, u32 time, Spikes* spikes,
                                Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(LAYER_LEARNING_STEP_FORCE_SPIKE);

    LayerStepData data;
    data.type = LAYER_TASK_LEARNING_STEP_FORCE_SPIKE;
    data.spikes = spikes;

    _layer_run(layer, time, memory, pool, &data);

    TIMING_COUNTER_END(LAYER_LEARNING_STEP_FORCE_SPIKE);
}
