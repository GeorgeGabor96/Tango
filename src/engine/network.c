internal Network*
network_create(State* state, const char* name) {
    check(state != NULL, "state is NULL");
    check(name != NULL, "name is NULL");

    Network* network = NULL;

    network = (Network*)memory_push(state->permanent_storage, sizeof(*network));
    check_memory(network);

    network->name = string_create(state->permanent_storage, name);
    check(network->name != NULL, "network->name is NULL");

    network->layers = NULL;
    network->in_layers = NULL;
    network->out_layers = NULL;

    network->n_layers = 0;
    network->n_in_layers = 0;
    network->n_out_layers = 0;

    network->neurons = NULL;
    network->synapses = NULL;
    network->n_neurons = 0;
    network->n_synapses = 0;

    return network;

    error:
    return NULL;
}


internal bool
network_build(State* state, Network* network) {
    check(state != NULL, "state is NULL");
    check(network != NULL, "network is NULL");

    u32 n_neurons = 0;
    u32 n_synapses = 0;

    NetworkLayerLink* layer_it = NULL;
    LayerLink* in_layer_it = NULL;
    Layer* layer = NULL;
    Layer* input = NULL;
    f32 chance = 0.0f;

    for (layer_it = network->layers; layer_it != NULL; layer_it = layer_it->next) {
        layer = layer_it->layer;
        n_neurons += layer->n_neurons;

        in_layer_it = NULL;
        for (in_layer_it = layer->inputs; in_layer_it != NULL; in_layer_it = in_layer_it->next) {
            if (in_layer_it->chance >= 1.0f) {
                n_synapses += layer->n_neurons * in_layer_it->layer->n_neurons;
            } else {
                chance = math_clip_f32(in_layer_it->chance + 0.1f, 0.0f, 1.0f);
                n_synapses += layer->n_neurons * chance * in_layer_it->layer->n_neurons;
            }
        }
    }

    network->neurons = (Neuron*)memory_push(state->permanent_storage, sizeof(Neuron) * n_neurons);
    check_memory(network->neurons);
    network->synapses = (Synapse*)memory_push(state->permanent_storage, sizeof(Synapse) * n_synapses);
    check_memory(network->synapses);
    network->n_neurons = n_neurons;
    network->n_synapses = n_synapses;

    // allocate neurons in each layer
    u32 neuron_offset = 0;
    u32 neuron_i = 0;
    Neuron* neuron = NULL;
    for (layer_it = network->layers; layer_it != NULL; layer_it = layer_it->next) {
        layer = layer_it->layer;
        layer->neuron_start_i = neuron_offset;
        neuron_offset += layer->n_neurons;
        layer->neuron_end_i = neuron_offset;
        layer_init_neurons(layer, network->neurons);

    }
    check(neuron_offset <= network->n_neurons, "Used more neurons than were allocated");

    // allocate synapses to each pair of layers
    u32 synapse_offset = 0;
    for (layer_it = network->layers; layer_it != NULL; layer_it = layer_it->next) {
        layer = layer_it->layer;

        for (in_layer_it = layer->inputs; in_layer_it != NULL; in_layer_it = in_layer_it->next) {
            synapse_offset = layer_link_synapses(state, layer, in_layer_it, network->neurons, network->synapses, synapse_offset);
        }
    }
    check(synapse_offset <= network->n_synapses, "Used more synapses than were allocated");

    return TRUE;
    error:
    return FALSE;
}


internal void
network_show(Network* network) {
    check(network != NULL, "network is NULL");
    u32 i = 0;
    Layer* layer = NULL;
    NetworkLayerLink* it = NULL;

    printf("-----------------------NETWORK---------------------\n");
    printf("Name: %s\n\n", string_get_c_str(network->name));

    printf("Layers:\n");
    for (it = network->layers; it != NULL; it = it->next) {
        layer = it->layer;
        layer_show(layer, network->neurons);
    }
    printf("Number of layers: %u\n\n", network->n_layers);

    printf("Input Layers: ");
    for (it = network->in_layers; it != NULL; it = it->next)
        printf("%s, ", string_get_c_str(it->layer->name));
    printf("\nNumber of input layers: %u\n\n", network->n_in_layers);

    printf("Output Layers: ");
    for (it = network->out_layers; it != NULL; it = it->next)
        printf("%s, ", string_get_c_str(it->layer->name));
    printf("\nNumber of output layers: %u\n\n", network->n_out_layers);

    printf("Number of neurons: %u\n", network->n_neurons);
    printf("Number of synapses: %u\n", network->n_synapses);
    printf("-----------------------NETWORK---------------------\n");

    error:
    return;
}


internal void
network_add_layer(State* state, Network* network, Layer* layer,
                  bool is_input, bool is_output) {
    check(state != NULL, "state is NULL");
    check(network != NULL, "network is NULL");
    check(layer != NULL, "layer is NULL");

    NetworkLayerLink* link = (NetworkLayerLink*)memory_push(state->permanent_storage, sizeof(*link));
    check_memory(link);
    link->layer = layer;
    link->next = network->layers ? network->layers : NULL;
    network->layers = link;
    // TODO: ned to add at the end??
    ++(network->n_layers);

    if (is_input == TRUE) {
        link = (NetworkLayerLink*)memory_push(state->permanent_storage, sizeof(*link));
        link->layer = layer;
        link->next = network->in_layers ? network->in_layers : NULL;
        network->in_layers = link;
        ++(network->n_in_layers);
    }

    if (is_output == TRUE) {
        link = (NetworkLayerLink*)memory_push(state->permanent_storage, sizeof(*link));
        link->layer = layer;
        link->next = network->out_layers ? network->out_layers : NULL;
        network->out_layers = link;
        ++(network->n_out_layers);
    }

    error:
    return;
}


internal void
_network_step(Network* network, Inputs* inputs, u32 time, Memory* memory, ThreadPool* pool,
              Mode mode) {
    check(network != NULL, "network is NULL");
    check(inputs != NULL, "inputs is NULL");
    check(memory != NULL, "memory is NULL");
    check(pool != NULL, "pool is NULL");
    check(network->n_in_layers == inputs->n_inputs,
          "network->n_in_layers is %u, inputs->n_inputs is %u, should be equal",
          network->n_in_layers, inputs->n_inputs);
    check(mode == MODE_INFER || mode == MODE_LEARNING,
          "Unknown mode %u (%s)", mode, mode_get_c_str(mode));

    Layer* layer = NULL;
    Input* input = NULL;
    NetworkLayerLink* it = NULL;
    u32 i = 0;

    // NOTE: Assume that the order of inputs are the same as the order of input layers in
    // NOTE: the network
    for (i = 0, it = network->in_layers; it != NULL; ++i, it = it->next) {
        input = inputs->inputs + i;
        layer = it->layer;

        if (mode == MODE_INFER) {
            if (input->type == INPUT_SPIKES)
                layer_step_force_spike(layer, network->neurons, network->synapses,
                                       time, &(input->spikes), memory, pool);
            else if (input->type == INPUT_CURRENT)
                layer_step_inject_current(layer, network->neurons, network->synapses,
                                          time, &(input->currents), memory, pool);
            else
                log_error("Unknown network input type %d", input->type);
        } else if (mode == MODE_LEARNING) {
            if (input->type == INPUT_SPIKES)
                layer_learning_step_force_spike(layer, network->neurons, network->synapses,
                                                time, &(input->spikes), memory, pool);
            else if (input->type == INPUT_CURRENT)
                layer_learning_step_inject_current(layer, network->neurons, network->synapses,
                                                   time, &(input->currents), memory, pool);
            else
                log_error("Unknown network input type %d", input->type);
        }
        layer->it_ran = TRUE;
    }

    for (it = network->layers; it != NULL; it = it->next) {
        layer = it->layer;
        if (layer->it_ran == FALSE)
            if (mode == MODE_INFER)
                layer_step(layer, network->neurons, network->synapses, time, memory, pool);
            else if (mode == MODE_LEARNING)
                layer_learning_step(layer, network->neurons, network->synapses, time, memory, pool);
    }

    error:
    return;
}


internal void
network_infer(Network* network, Inputs* inputs, u32 time, Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(NETWORK_INFER);
    _network_step(network, inputs, time, memory, pool, MODE_INFER);
    TIMING_COUNTER_END(NETWORK_INFER);
}


internal void
network_learn(Network* network, Inputs* inputs, u32 time, Memory* memory, ThreadPool* pool) {
    TIMING_COUNTER_START(NETWORK_LEARN);
    _network_step(network, inputs, time, memory, pool, MODE_LEARNING);
    TIMING_COUNTER_END(NETWORK_LEARN);
}


internal void
network_clear(Network* network) {
    check(network != NULL, "network is NULL");
    NetworkLayerLink* it = NULL;

    for (it = network->layers; it != NULL; it = it->next)
        layer_clear(it->layer, network->neurons, network->synapses);

    error:
    return;
}


internal f32*
network_get_layer_voltages(State* state, Network* network, u32 i) {
    check(state != NULL, "state is NULL");
    check(network != NULL, "network is NULL");
    check(i < network->n_layers, "i >= network->n_layers");

    u32 j = 0;
    NetworkLayerLink* it = NULL;
    Layer* layer = NULL;

    for (j = 0, it = network->layers; it != NULL; ++j, it = it->next) {
        if (j == i) {
            layer = it->layer;
            break;
        }
    }

    f32* voltages = layer_get_voltages(state->transient_storage, layer, network->neurons);
    return voltages;

    error:
    return NULL;
}


internal bool*
network_get_layer_spikes(State* state, Network* network, u32 i) {
    check(state != NULL, "state is NULL");
    check(network != NULL, "network is NULL");
    check(i < network->n_layers, "i >= network->n_layers");

    u32 j = 0;
    NetworkLayerLink* it = NULL;
    Layer* layer = NULL;

    for (j = 0, it = network->layers; it != NULL; ++j, it = it->next) {
        if (j == i) {
            layer = it->layer;
            break;
        }
    }

    bool* spikes = layer_get_spikes(state->transient_storage, layer, network->neurons);
    return spikes;

    error:
    return NULL;
}


internal u32
network_get_layer_idx(Network* net, Layer* layer) {
    check(net != NULL, "net is NULL");
    check(layer != NULL, "layer is NULL");

    u32 i = 0;
    NetworkLayerLink* it = NULL;

    for (i = 0, it = net->layers; it != NULL; ++i, it = it->next)
        if (it->layer == layer) return i;

    error:
    return (u32) -1;
}
