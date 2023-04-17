// TODO: for extreme privacy we can return a handle and internally get the exp from it

// TODO: most of the function return a b32, but they return TRUE always, maybe in the future it will make sense to actually be posible to return FALSE

void* tango_create(u32 n_workers, u32 seed, const char* output_folder) {
    Experiment* exp = experiment_create(n_workers, seed, output_folder);
    exp->network = network_create(exp->permanent_memory);
    return exp;
}


void tango_destroy(void* instance) {
    Experiment* exp = (Experiment*)instance;
    experiment_destroy(exp);
}


b32 tango_create_neuron_cls_lif_refract(void* instance,
                                        const char* name, u32 refract_time) {
    Experiment* exp = (Experiment*)instance;
    String* name_s = string_create(exp->permanent_memory, name);

    NeuronCls* cls = neuron_cls_create_lif_refract(exp->permanent_memory, name_s, refract_time);
    network_add_neuron_cls(exp->network, cls, exp->permanent_memory);
    return TRUE;
}


b32 tango_create_synapse_cls(void* instance,
                             const char* name, SynapseType type,
                             f32 rev_potential, f32 amp,
                             f32 tau_ms, u32 delay) {
    Experiment* exp = (Experiment*)instance;
    String* name_s = string_create(exp->permanent_memory, name);
    SynapseCls* cls = synapse_cls_create(exp->permanent_memory, name_s, type,
                                         rev_potential, amp, tau_ms, delay);
    network_add_synapse_cls(exp->network, cls, exp->permanent_memory);
    return TRUE;
}


b32 tango_create_layer(void* instance,
                       const char* layer_name, LayerType type, u32 n_neurons,
                       const char* neuron_cls_name, b32 is_input, b32 is_output) {
    Experiment* exp = (Experiment*)instance;

    NetworkLayerLink* it = NULL;
    for (it = exp->network->layers.first; it != NULL; it = it->next) {
        if (string_equal_c_str(it->layer->name, layer_name)) {
            log_warning("Layer with name %s already present. Current layer is not added", layer_name);
            return FALSE;
        }
    }

    String* neuron_cls_name_s = string_create(exp->transient_memory, neuron_cls_name);
    NeuronCls* cls = network_get_neuron_cls(exp->network, neuron_cls_name_s);

    String* layer_name_s = string_create(exp->permanent_memory, layer_name);

    Layer* layer = layer_create(exp->permanent_memory, layer_name_s, type, n_neurons, cls);
    network_add_layer(exp->network, layer, is_input, is_output, exp->permanent_memory);
    return TRUE;
}


b32 tango_link_layers(void* instance,
                      const char* layer_name, const char* in_layer_name,
                      const char* synapse_cls_name, f32 synapse_weight,
                      f32 connect_chance) {
    // NOTE: this is okay for now but in the future we should create strings and
    // NOTE: use that stack point reset trick to create them
    if (strcmp(layer_name, in_layer_name) == 0)
        log_warning("The layer_name '%s' == in_layer_name '%s'. Make sure this is what you want", layer_name, in_layer_name);

    Experiment* exp = (Experiment*)instance;
    String* name = string_create(exp->transient_memory, synapse_cls_name);
    SynapseCls* cls = network_get_synapse_cls(exp->network, name);

    name = string_create(exp->transient_memory, layer_name);
    Layer* layer = network_get_layer(exp->network, name);

    name = string_create(exp->transient_memory, in_layer_name);
    Layer* in_layer = network_get_layer(exp->network, name);

    b32 result = layer_link(layer, in_layer, cls, synapse_weight, connect_chance,
                            exp->permanent_memory);
    return result;
}


b32 tango_build_network(void* instance) {
    Experiment* exp = (Experiment*)instance;
    b32 result = network_build(exp->network, exp->permanent_memory, exp->random);
    return result;
}


void tango_show_network(void* instance) {
    Experiment* exp = (Experiment*)instance;
    network_show(exp->network);
}


b32 tango_create_data_constant_current(void* instance, f32 value,
                                       u32 n_samples, u32 sample_duration) {
    Experiment* exp = (Experiment*)instance;
    if (exp->data)
        log_warning("Data gen already set for experiment. Will overwrite");
    DataGen* data = data_gen_create_constant_current(exp->permanent_memory,
                                              value, n_samples, sample_duration);
    experiment_set_data_gen(exp, data);

    return TRUE;
}


b32 tango_create_data_random_spikes(void* instance, f32 chance,
                                    u32 n_samples, u32 sample_duration) {
    Experiment* exp = (Experiment*)instance;
    if (exp->data)
        log_warning("Data gen already set for experiment. Will overwrite");
    DataGen* data = data_gen_create_random_spikes(exp->permanent_memory,
                                                  exp->random,
                                                  chance, n_samples, sample_duration);
    experiment_set_data_gen(exp, data);
    return TRUE;
}


b32 tango_create_data_spike_pulses(void* instance,
                                   u32 n_samples, u32 sample_duration,
                                   u32 first_pulse_time, u32 pulse_duration,
                                   u32 between_pulses_duration,
                                   f32 pulse_spike_chance, f32 between_pulses_spike_chance) {
    Experiment* exp = (Experiment*)instance;
    if (exp->data)
        log_warning("Data gen already set for experiment. Will overwrite");
    DataGen* data = data_gen_create_spike_pulses(
        exp->permanent_memory, exp->random, n_samples, sample_duration, first_pulse_time, pulse_duration, between_pulses_duration,
        pulse_spike_chance, between_pulses_spike_chance);
    experiment_set_data_gen(exp, data);
    return TRUE;
}


b32 tango_create_callback_meta_dumper(void* instance) {
    Experiment* exp = (Experiment*)instance;
    Callback* dumper = callback_meta_dumper_create(exp->permanent_memory,
                                                   exp->output_folder,
                                                   exp->network);
    experiment_add_callback(exp, dumper);
    return TRUE;
}


void tango_learn(void* instance) {
    Experiment* exp = (Experiment*)instance;
    experiment_learn(exp);
}


void tango_infer(void* instance) {
    Experiment* exp = (Experiment*)instance;
    experiment_infer(exp);
}
