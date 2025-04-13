#include "tango.h"


int main() {
    Experiment* exp = experiment_create(0, 723104, "D:\\repos\\Tango\\outputs\\147_background_activity_13_04_2025\\6_connection_prob");
    Memory* memory = exp->permanent_memory;
    DataGen* data_gen = data_gen_create_background_activity(memory, exp->random, 0.005f, 0.005f, 100, 1000);

    Network* net = network_create(memory);
    NeuronCls* n_cls = neuron_cls_create_lif_refract(memory, string_create(memory, "if"), 5);

    Layer* in = layer_create(memory, string_create(memory, "in"), LAYER_DENSE, 100, n_cls);
    Layer* background = layer_create(memory, string_create(memory, "background"), LAYER_DENSE, 100, n_cls);
    Layer* hidden = layer_create(memory, string_create(memory, "hidden"), LAYER_DENSE, 100, n_cls);
    Layer* out = layer_create(memory, string_create(memory, "out"), LAYER_DENSE, 100, n_cls);

    SynapseCls* s_cls = synapse_cls_create(memory, string_create(memory, "s_cls"), SYNAPSE_CONDUCTANCE, 0.0f, 1.5, 5, 5);
    synapse_cls_add_learning_rule_step(s_cls, -1.0f, 1.0f, 20, 0.1f, 20, -0.1f);

    SynapseCls* s_cls_background_population = synapse_cls_create(memory, string_create(memory, "s_cls_background"), SYNAPSE_CONDUCTANCE, 0.0f, 1.5, 5, 5);

    network_add_neuron_cls(net, n_cls, memory);
    network_add_synapse_cls(net, s_cls, memory);
    network_add_synapse_cls(net, s_cls_background_population, memory);

    LayerLinkSynapseInitMeta* synapse_connect_info = create_layer_init_synapse_init_meta(memory, -1.0, 1.0, 0.8, 0.8);
    layer_link(hidden, in, s_cls, synapse_connect_info, memory);
    layer_link(hidden, background, s_cls_background_population, synapse_connect_info, memory);
    layer_link(out, hidden, s_cls, synapse_connect_info, memory);

    network_add_layer(net, in, TRUE, FALSE, memory);
    network_add_layer(net, background, TRUE, FALSE, memory);
    network_add_layer(net, hidden, FALSE, FALSE, memory);
    network_add_layer(net, out, FALSE, TRUE, memory);
    network_build(net, memory, exp->random);
    network_show(net);

    Callback* cb_meta = callback_meta_dumper_create(memory, exp->output_folder, net);
    Callback* cb_spikes = callback_spikes_dumper_create(memory, exp->output_folder, net);
    Callback* cb_weights = callback_weights_dumper_create(memory, 1, 5000, exp->output_folder, net);
    Callback* cb_data = callback_network_data_dumper_create(memory, exp->output_folder, net);
    Callback* cb_stdp = callback_stdp_v1_create(memory, net, 10);

    experiment_set_network(exp, net);
    experiment_set_data_gen(exp, data_gen);

    experiment_add_callback(exp, cb_meta);
    experiment_add_callback(exp, cb_spikes);
    experiment_add_callback(exp, cb_weights);
    //experiment_add_callback(exp, cb_data);
    experiment_add_callback(exp, cb_stdp);

    experiment_set_learning(exp, TRUE);

    experiment_run(exp);

    experiment_destroy(exp);

    // NOTE: currently I can alternate between input on and off, I get sync activity because a neuron on a layer gets more or less the same input Current as the others becuase
    // its fully connected

    return 0;
}