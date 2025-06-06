internal Network*
network_create(Memory* memory) {
    check(memory != NULL, "memory is NULL");

    Network* network = NULL;

    network = (Network*)memory_push(memory, sizeof(*network));
    check_memory(network);

    network->neuron_classes = NULL;
    network->synapse_classes = NULL;

    network->n_neuron_cls = 0;
    network->n_synapse_cls = 0;

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
            if (in_layer_it->synapse_meta->connect_chance >= 1.0f) {
                n_max_synapses += layer->n_neurons * in_layer_it->layer->n_neurons;
            } else {
                chance = math_clip_f32(in_layer_it->synapse_meta->connect_chance + 0.1f, 0.0f, 1.0f);
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

    // allocate synapses to each pair of layers and link them
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
network_add_neuron_cls(Network* network, NeuronCls* cls, Memory* memory) {
    check(network != NULL, "network is NULL");
    check(cls != NULL, "cls is NULL");
    check(memory != NULL, "memory is NULL");
    check(network->is_built == FALSE, "Trying to add a neuron class in a built network")

    NetworkNeuronClsLink* link = (NetworkNeuronClsLink*)memory_push(memory, sizeof(*link));
    check_memory(link);
    link->cls = cls;
    link->next = network->neuron_classes ? network->neuron_classes : NULL;
    network->neuron_classes = link;
    network->n_neuron_cls += 1;

    error:
    return;
}


internal void
network_add_synapse_cls(Network* network, SynapseCls* cls, Memory* memory) {
    check(network != NULL, "network is NULL");
    check(cls != NULL, "cls is NULL");
    check(memory != NULL, "memory is NULL");
    check(network->is_built == FALSE, "Trying to add a synapse class in a built network")

    NetworkSynapseClsLink* link = (NetworkSynapseClsLink*) memory_push(memory, sizeof(*link));
    check_memory(link);
    link->cls = cls;
    link->next = network->synapse_classes ? network->synapse_classes : NULL;
    network->synapse_classes = link;
    network->n_synapse_cls += 1;

    error:
    return;
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


internal NeuronCls* network_get_neuron_cls(Network* network, String* neuron_cls_name) {
    check(network != NULL, "network is NULL");
    check(neuron_cls_name != NULL, "neuron_cls_name");

    NetworkNeuronClsLink* it = NULL;
    for (it = network->neuron_classes; it != NULL; it = it->next) {
        if (string_equal(neuron_cls_name, it->cls->name))
            return it->cls;
    }

    error:
    return NULL;
}


internal SynapseCls* network_get_synapse_cls(Network* network, String* synapse_cls_name) {
    check(network != NULL, "network is NULL");
    check(synapse_cls_name != NULL, "synapse_cls_name");

    NetworkSynapseClsLink* it = NULL;
    for (it = network->synapse_classes; it != NULL; it = it->next) {
        if (string_equal(synapse_cls_name, it->cls->name))
            return it->cls;
    }

    error:
    return NULL;
}


internal Layer* network_get_layer(Network* network, String* layer_name) {
    check(network != NULL, "network is NULL");
    check(layer_name != NULL, "layer_name is NULL");

    NetworkLayerLink* it = NULL;
    for (it = network->layers.first; it != NULL; it = it->next) {
        if (string_equal(layer_name, it->layer->name))
            return it->layer;
    }

    error:
    return NULL;
}

internal void network_set_learning(Network* network, b32 value) {
    check(network != NULL, "network is NULL");
    check(value == FALSE || value == TRUE, "invalid value for b32 %d", value);

    for (NetworkSynapseClsLink* link = network->synapse_classes; link != NULL; link = link->next) {
        link->cls->learning_info.enable = value;
    }

    error:
    return;
}

internal void
network_step(Network* network, Inputs* inputs, u32 time, Memory* memory, ThreadPool* pool) {
    check(network != NULL, "network is NULL");
    check(network->is_built == TRUE, "network should be built");
    check(inputs != NULL, "inputs is NULL");
    check(memory != NULL, "memory is NULL");
    check(pool != NULL, "pool is NULL");
    check(network->n_in_layers == inputs->n_inputs,
          "network->n_in_layers is %u, inputs->n_inputs is %u, should be equal",
          network->n_in_layers, inputs->n_inputs);

    Layer* layer = NULL;
    Input* input = NULL;
    NetworkLayerLink* it = NULL;
    u32 i = 0;

    // NOTE: Assume that the order of inputs are the same as the order of input layers in
    // NOTE: the network
    for (i = 0, it = network->in_layers.first; it != NULL; ++i, it = it->next) {
        input = inputs->inputs + i;
        layer = it->layer;

        if (input->type == INPUT_SPIKES) {
            layer_step_force_spike(layer, time, &(input->spikes), memory, pool);
        } else if (input->type == INPUT_CURRENT) {
            layer_step_inject_current(layer,
                                        time, &(input->currents), memory, pool);
        } else {
            log_error("Unknown network input type %d", input->type);
        }
        layer->it_ran = TRUE;
    }

    for (it = network->layers.first; it != NULL; it = it->next) {
        layer = it->layer;
        if (layer->it_ran == FALSE) {
            layer_step(layer, time, memory, pool);
        }
    }

    error:
    return;
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

internal void
network_set_neuron_history(Network* network, u32 sample_duration, Memory* memory)
{
    check(network != NULL, "network is NULL");
    check(sample_duration > 0, "sample_duration is 0");
    NetworkLayerLink* it = NULL;

    for (it = network->layers.first; it != NULL; it = it->next)
    {
        layer_set_neuron_history(it->layer, sample_duration, memory);
    }

    error:
    return;
}