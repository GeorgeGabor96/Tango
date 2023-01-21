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

    layer->n_neurons = n_neurons;
    // TODO: should I just keep pointers to neurons and remove the init?
    layer->neurons = (Neuron*)memory_push(state->permanent_storage, layer->n_neurons * sizeof(Neuron));
    check_memory(layer->neurons);
    for (i = 0; i < layer->n_neurons; ++i)
        neuron_init(layer->neurons + i, cls);

    memset(layer->in_layers, 0, sizeof(Layer*) * LAYER_N_MAX_IN_LAYERS);

    layer->name = string_create(state->permanent_storage, name);
    check_memory(layer->name);
    layer->type = type;
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
    Neuron* neurons = layer_task->layer->neurons;
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

    for (u32 i = 0; i < layer->n_neurons; ++i)
        neuron_clear(layer->neurons + i);

    layer->it_ran = FALSE;

    error:
    return;
}


internal u32
layer_get_n_in_synapses(Layer* layer) {
    u32 n_in_synapses = 0;
    u32 i = 0;
    u32 j = 0;
    Neuron* neuron = NULL;

    check(layer != NULL, "Layer is NULL");

    for (i = 0; i < layer->n_neurons; ++i) {
        neuron = layer->neurons + i;
        for (j = 0; j < neuron->n_in_synapse_arrays; ++j) {
            n_in_synapses += neuron->in_synapse_arrays[j]->length;
        }
    }

    error:
    return n_in_synapses;
}


internal void
layer_show(Layer* layer) {
    check(layer != NULL, "layer is NULL");

    u32 i = 0;
    u32 j = 0;
    u32 n_in_synapses = layer_get_n_in_synapses(layer);

    printf("-------------------\n");
    printf("Name: %s\n", string_get_c_str(layer->name));
    printf("Type: %s\n", layer_type_get_c_str(layer->type));
    printf("Number of neurons %u of type %s\n",
           layer->n_neurons,
           string_get_c_str(layer->neurons[0].cls->name));
    printf("Number of input synapses %u\n", n_in_synapses);
    printf("Input layers: ");
    for (i = 0; i < layer->n_in_layers; ++i)
        printf("%s, ", string_get_c_str(layer->in_layers[i]->name));

    printf("\n-------------------\n\n");
    error:
    return;
}


internal bool
_layer_link_dense(State* state,
                 Layer* layer, Layer* in_layer,
                 SynapseCls* cls, f32 weight, f32 chance) {
    u32 neuron_i = 0;
    u32 in_neuron_i = 0;
    u32 synapse_i = 0;
    u32 in_synapses_init = 0;
    Neuron* neuron = NULL;
    Neuron* in_neuron = NULL;
    Synapse* synapse = NULL;
    InSynapseArray* in_synapses = NULL;
    bool status = FALSE;
    sz synapse_size = synapse_size_with_cls(cls);

    OutSynapseArray* out_synapses = NULL;
    OutSynapseArray** out_synapses_in_layer = memory_push(state->transient_storage,
                                                          in_layer->n_neurons * sizeof(OutSynapseArray*));
    check_memory(out_synapses_in_layer);

    // INIT the output synapses for each of the input neurons
    for (neuron_i = 0; neuron_i < in_layer->n_neurons; ++neuron_i) {
        out_synapses = memory_push(state->permanent_storage,
                                   sizeof(OutSynapseArray) + sizeof(Synapse*) * layer->n_neurons);
        check_memory(out_synapses);
        out_synapses->length = 0;
        out_synapses->synapses = (Synapse**)(out_synapses + 1);
        out_synapses_in_layer[neuron_i] = out_synapses;
    }

    for (neuron_i = 0; neuron_i < layer->n_neurons; ++neuron_i) {
        neuron = layer->neurons + neuron_i;

        // ALLOC all input synapses
        in_synapses = (InSynapseArray*)memory_push(state->permanent_storage,
                                        sizeof(InSynapseArray) + synapse_size * in_layer->n_neurons);
        check_memory(in_synapses);
        in_synapses->length = in_layer->n_neurons;
        in_synapses->synapse_size = synapse_size;
        in_synapses->synapses = (Synapse*)(in_synapses + 1);

        in_synapses_init = 0;

        for (in_neuron_i = 0;
             in_neuron_i < in_layer->n_neurons;
             ++in_neuron_i) {

            if (random_get_chance_f32() > chance)
                continue;

            in_neuron = in_layer->neurons + in_neuron_i;

            synapse = in_synapse_array_get(in_synapses, in_synapses_init);
            ++(in_synapses_init);

            synapse_init(synapse, cls, weight);
            synapse->in_neuron = in_neuron;
            synapse->out_neuron =  neuron;

            // NOTE: save the synapse in the output synapses of the input neuron
            out_synapses = out_synapses_in_layer[in_neuron_i];
            out_synapses->synapses[out_synapses->length] = synapse;
            ++(out_synapses->length);

            // THIS should never happen
            check(out_synapses->length <= layer->n_neurons,
                  "out_synapses->length > layer->n_neurons");
        }
        in_synapses->length = in_synapses_init;
        neuron_add_in_synapse_array(neuron, in_synapses);
    }

    for (neuron_i = 0; neuron_i < in_layer->n_neurons; ++neuron_i) {
        neuron = in_layer->neurons + neuron_i;
        out_synapses = out_synapses_in_layer[neuron_i];
        neuron_add_out_synapse_array(neuron, out_synapses);
    }

    status = TRUE;
    error:
    return status;
}


internal bool
layer_link(State* state, Layer* layer, Layer* input_layer, SynapseCls* cls, f32 weight, f32 chance) {
    bool status = FALSE;
    check(state != NULL, "state is NULL");
    check(layer != NULL, "layer is NULL");
    check(input_layer != NULL, "input_layer is NULL");
    check(cls != NULL, "cls is NULL");
    check(chance >= 0.0f && chance <= 1.0f, "chance should be in [0, 1]");

    if (layer->type == LAYER_DENSE) {
        status = _layer_link_dense(state, layer, input_layer, cls, weight, chance);
        check(status == TRUE, "couldn't link layers %s and %s",
              string_get_c_str(layer->name), string_get_c_str(input_layer->name));
    } else {
        log_error("Unknown layer type %u", layer->type);
    }

    // NOTE: Save reference to the input layer
    check(layer->n_in_layers < LAYER_N_MAX_IN_LAYERS, "too many input layers");
    layer->in_layers[layer->n_in_layers] = input_layer;
    ++(layer->n_in_layers);

    error:
    return status;
}


internal f32*
layer_get_voltages(Memory* memory, Layer* layer) {
    check(memory != NULL, "memory is NULL");
    check(layer != NULL, "layer is NULL");

    f32* voltages = (f32*)memory_push(memory, sizeof(f32) * layer->n_neurons);
    check_memory(voltages);

    for (u32 i = 0; i < layer->n_neurons; ++i)
        voltages[i] = layer->neurons[i].voltage;

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

    for (u32 i = 0; i < layer->n_neurons; ++i)
        pscs[i] = layer->neurons[i].epsc + layer->neurons[i].ipsc;

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

    for (u32 i = 0; i < layer->n_neurons; ++i)
        epscs[i] = layer->neurons[i].epsc;

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

    for (u32 i = 0; i < layer->n_neurons; ++i)
        ipscs[i] = layer->neurons[i].ipsc;

    return ipscs;

    error:
    return NULL;
}


internal bool*
layer_get_spikes(Memory* memory, Layer* layer) {
    check(memory != NULL, "memory is NULL");
    check(layer != NULL, "layer is NULL");

    bool* spikes = (bool*)memory_push(memory, sizeof(bool) * layer->n_neurons);
    check_memory(spikes);

    for (u32 i = 0; i < layer->n_neurons; ++i)
        spikes[i] = layer->neurons[i].spike;

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


internal u32
layer_get_neuron_idx(Layer* layer, Neuron* neuron) {
    check(layer != NULL, "layer is NULL");
    check(neuron != NULL, "neuron is NULL");
    check(neuron >= layer->neurons && neuron <= layer->neurons + layer->n_neurons - 1,
          "neuron doesn't belong to this layer");

    u32 idx = (u32)(neuron - layer->neurons);
    return idx;

    error:
    return (u32)-1;
}
