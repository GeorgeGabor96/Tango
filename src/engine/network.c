internal Network*
network_create(Memory* memory, const char* name) {
    check(memory != NULL, "memory is NULL");
    check(name != NULL, "name is NULL");

    Network* network = NULL;

    network = (Network*)memory_push(memory, sizeof(*network));
    check_memory(network);

    network->name = string_create(memory, name);
    check(network->name != NULL, "network->name is NULL");

    network->layers.first = NULL;
    network->layers.last = NULL;
    network->in_layers.first = NULL;
    network->in_layers.last = NULL;
    network->out_layers.first = NULL;
    network->out_layers.last = NULL;

    network->n_layers = 0;
    network->n_in_layers = 0;
    network->n_out_layers = 0;

    network->neurons = NULL;
    network->synapses = NULL;
    network->n_neurons = 0;
    network->n_synapses = 0;

    network->is_built = FALSE;

    return network;

    error:
    return NULL;
}


internal b32
network_build(Network* network, Memory* memory, Random* random) {
    check(network != NULL, "network is NULL");
    check(network->is_built == FALSE, "layers cannot be added if the network is built");
    check(memory != NULL, "memory is NULL");
    check(random != NULL, "random is NULL");

    u32 n_neurons = 0;
    u32 n_max_synapses = 0;

    NetworkLayerLink* layer_it = NULL;
    LayerLink* in_layer_it = NULL;
    Layer* layer = NULL;
    Layer* input = NULL;
    f32 chance = 0.0f;

    for (layer_it = network->layers.first; layer_it != NULL; layer_it = layer_it->next) {
        layer = layer_it->layer;
        n_neurons += layer->n_neurons;

        in_layer_it = NULL;
        for (in_layer_it = layer->inputs; in_layer_it != NULL; in_layer_it = in_layer_it->next) {
            if (in_layer_it->chance >= 1.0f) {
                n_max_synapses += layer->n_neurons * in_layer_it->layer->n_neurons;
            } else {
                chance = math_clip_f32(in_layer_it->chance + 0.1f, 0.0f, 1.0f);
                n_max_synapses += (u32)((f32)layer->n_neurons * chance * (f32)in_layer_it->layer->n_neurons);
            }
        }
    }

    network->neurons = (Neuron*)memory_push(memory, sizeof(Neuron) * n_neurons);
    check_memory(network->neurons);
    network->synapses = (Synapse*)memory_push(memory, sizeof(Synapse) * n_max_synapses);
    check_memory(network->synapses);

    // allocate neurons in each layer
    u32 neuron_offset = 0;
    u32 neuron_i = 0;
    Neuron* neuron = NULL;
    for (layer_it = network->layers.first; layer_it != NULL; layer_it = layer_it->next) {
        layer = layer_it->layer;
        layer->neurons = network->neurons + neuron_offset;
        neuron_offset += layer->n_neurons;
        layer_init_neurons(layer);

    }
    check(neuron_offset <= n_neurons, "Used more neurons than were allocated");

    // allocate synapses to each pair of layers
    u32 synapse_offset = 0;
    for (layer_it = network->layers.first; layer_it != NULL; layer_it = layer_it->next) {
        layer = layer_it->layer;

        for (in_layer_it = layer->inputs; in_layer_it != NULL; in_layer_it = in_layer_it->next) {
            synapse_offset = layer_link_synapses(layer, in_layer_it, network->synapses, synapse_offset, memory, random);
        }
    }
    check(synapse_offset <= n_max_synapses, "Used more synapses than were allocated");

    network->n_neurons = n_neurons;
    network->n_synapses = synapse_offset;

    network->is_built = TRUE;

    return TRUE;
    error:
    return FALSE;
}


internal void
network_show(Network* network) {
    check(network != NULL, "network is NULL");
    check(network->is_built == TRUE, "network should be built");

    u32 i = 0;
    Layer* layer = NULL;
    NetworkLayerLink* it = NULL;

    printf("-----------------------NETWORK---------------------\n");
    printf("Name: %s\n\n", string_get_c_str(network->name));

    printf("Layers:\n");
    for (it = network->layers.first; it != NULL; it = it->next) {
        layer = it->layer;
        layer_show(layer);
    }
    printf("Number of layers: %u\n\n", network->n_layers);

    printf("Input Layers: ");
    for (it = network->in_layers.first; it != NULL; it = it->next)
        printf("%s, ", string_get_c_str(it->layer->name));
    printf("\nNumber of input layers: %u\n\n", network->n_in_layers);

    printf("Output Layers: ");
    for (it = network->out_layers.first; it != NULL; it = it->next)
        printf("%s, ", string_get_c_str(it->layer->name));
    printf("\nNumber of output layers: %u\n\n", network->n_out_layers);

    printf("Number of neurons: %u\n", network->n_neurons);
    printf("Number of synapses: %u\n", network->n_synapses);
    printf("-----------------------NETWORK---------------------\n");

    error:
    return;
}


internal b32
_network_link_layer(Memory* memory, NetworkLayerList* chain, Layer* layer) {
    NetworkLayerLink* link = (NetworkLayerLink*)memory_push(memory, sizeof(*link));
    check_memory(link);
    link->layer = layer;
    link->next = NULL;
    if (chain->first) {
        chain->last->next = link;
        chain->last = link;
    } else {
        chain->first = link;
        chain->last = link;
    }

    return TRUE;
    error:
    return FALSE;
}


internal void
network_add_layer(Network* network, Layer* layer,
                  b32 is_input, b32 is_output, Memory* memory) {
    check(network != NULL, "network is NULL");
    check(network->is_built == FALSE, "layers cannot be added if the network is built");
    check(layer != NULL, "layer is NULL");
    check(memory != NULL, "memory is NULL");

    b32 res = _network_link_layer(memory, &network->layers, layer);
    check(res == TRUE, "Couldn't link layer in network->layers");
    ++(network->n_layers);

    if (is_input == TRUE) {
        res = _network_link_layer(memory, &network->in_layers, layer);
        check(res == TRUE, "Couldn't link layer in network->in_layers");
        ++(network->n_in_layers);
    }

    if (is_output == TRUE) {
        res = _network_link_layer(memory, &network->out_layers, layer);
        check(res == TRUE, "Couldn't link layer in network->out_layers");
        ++(network->n_out_layers);
    }

    error:
    return;
}


internal void
_network_step(Network* network, Inputs* inputs, u32 time, Memory* memory, ThreadPool* pool,
              Mode mode) {
    check(network != NULL, "network is NULL");
    check(network->is_built == TRUE, "network should be built");
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
    for (i = 0, it = network->in_layers.first; it != NULL; ++i, it = it->next) {
        input = inputs->inputs + i;
        layer = it->layer;

        if (mode == MODE_INFER) {
            if (input->type == INPUT_SPIKES)
                layer_step_force_spike(layer, time, &(input->spikes), memory, pool);
            else if (input->type == INPUT_CURRENT)
                layer_step_inject_current(layer,
                                          time, &(input->currents), memory, pool);
            else
                log_error("Unknown network input type %d", input->type);
        } else if (mode == MODE_LEARNING) {
            if (input->type == INPUT_SPIKES)
                layer_learning_step_force_spike(layer,
                                                time, &(input->spikes), memory, pool);
            else if (input->type == INPUT_CURRENT)
                layer_learning_step_inject_current(layer,
                                                   time, &(input->currents), memory, pool);
            else
                log_error("Unknown network input type %d", input->type);
        }
        layer->it_ran = TRUE;
    }

    for (it = network->layers.first; it != NULL; it = it->next) {
        layer = it->layer;
        if (layer->it_ran == FALSE)
            if (mode == MODE_INFER)
                layer_step(layer, time, memory, pool);
            else if (mode == MODE_LEARNING)
                layer_learning_step(layer, time, memory, pool);
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
    check(network->is_built == TRUE, "network should be built");
    NetworkLayerLink* it = NULL;

    for (it = network->layers.first; it != NULL; it = it->next)
        layer_clear(it->layer);

    error:
    return;
}
