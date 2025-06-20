#include "tango.h"


int main() {
    Experiment* exp = experiment_create(0, 723104, "D:\\repos\\Tango\\outputs\\156-add-lateral-inhibition\\1_basic");
    Memory* memory = exp->permanent_memory;
    DataGen* data_gen = data_gen_create_background_activity(memory, exp->random, 0.005f, 0.005f, 100, 1000);

    Network* net = network_create(memory);
    NeuronCls* n_cls = neuron_cls_create_lif_refract(memory, string_create(memory, "if"), 5);
    NeuronCls* n_cls_inhi = neuron_cls_create_if_refract(memory, string_create(memory, "if_inhi"), 5);

    Layer* in = layer_create(memory, string_create(memory, "in"), LAYER_DENSE, 10, n_cls);
    Layer* background = layer_create(memory, string_create(memory, "background"), LAYER_DENSE, 10, n_cls);

    Layer* hidden = layer_create(memory, string_create(memory, "hidden"), LAYER_DENSE, 20, n_cls);
    Layer* hidden_inhi = layer_create(memory, string_create(memory, "hidden_inhi"), LAYER_DENSE, 5, n_cls_inhi);

    Layer* out = layer_create(memory, string_create(memory, "out"), LAYER_DENSE, 10, n_cls);
    Layer* out_inhi = layer_create(memory, string_create(memory, "out_inhi"), LAYER_DENSE, 3, n_cls_inhi);

    f32 max_w = 1.0f;
    f32 min_w = 0.0f;
    SynapseCls* s_cls = synapse_cls_create_exci(memory, string_create(memory, "s_cls_fast_exci"), SYNAPSE_VOLTAGE, 1);
    synapse_cls_add_learning_rule_rstdp_exponential(s_cls, min_w, max_w, 1.0f, -1.0f, 20);

    SynapseCls* s_cls_rstdp = synapse_cls_create_exci(memory, string_create(memory, "s_cls_fast_exci"), SYNAPSE_VOLTAGE, 1);
    synapse_cls_add_learning_rule_rstdp_exponential(s_cls_rstdp, min_w, max_w, 1.0f, -1.0f, 20);

    SynapseCls* s_cls_background_population = synapse_cls_create(memory, string_create(memory, "s_cls_background"), SYNAPSE_VOLTAGE, 0.0f, 1, 7, 5);
    SynapseCls* s_cls_exci_fast = synapse_cls_create_exci(memory, string_create(memory, "s_cls_fast_exci"), SYNAPSE_VOLTAGE, 1);
    SynapseCls* s_cls_inhi = synapse_cls_create_inhi(memory, string_create(memory, "s_cls_inhi"), SYNAPSE_VOLTAGE, 1);

    network_add_neuron_cls(net, n_cls, memory);
    network_add_synapse_cls(net, s_cls, memory);
    network_add_synapse_cls(net, s_cls_background_population, memory);
    network_add_synapse_cls(net, s_cls_rstdp, memory);

    network_add_synapse_cls(net, s_cls_exci_fast, memory);
    network_add_synapse_cls(net, s_cls_inhi, memory);

    LayerLinkSynapseInitMeta* synapse_connect_info = create_layer_init_synapse_init_meta(memory, min_w, max_w, 0.8, 0.8);
    layer_link(hidden, in, s_cls_rstdp, synapse_connect_info, memory);
    layer_link(hidden, background, s_cls_background_population, synapse_connect_info, memory);
    layer_link(out, hidden, s_cls_rstdp, synapse_connect_info, memory);

    layer_link(hidden_inhi, hidden, s_cls_exci_fast, synapse_connect_info, memory);
    layer_link(hidden, hidden_inhi, s_cls_inhi, synapse_connect_info, memory);
    layer_link(out, out_inhi, s_cls_exci_fast, synapse_connect_info, memory);
    layer_link(out_inhi, out, s_cls_inhi, synapse_connect_info, memory);

    network_add_layer(net, in, TRUE, FALSE, memory);
    network_add_layer(net, background, TRUE, FALSE, memory);
    network_add_layer(net, hidden, FALSE, FALSE, memory);
    network_add_layer(net, hidden_inhi, FALSE, FALSE, memory);
    network_add_layer(net, out, FALSE, TRUE, memory);
    network_add_layer(net, out_inhi, FALSE, FALSE, memory);

    network_build(net, memory, exp->random);
    network_show(net);

    Callback* cb_meta = callback_meta_dumper_create(memory, exp->output_folder, net);
    Callback* cb_spikes = callback_spikes_dumper_create(memory, exp->output_folder, net);
    Callback* cb_weights = callback_weights_dumper_create(memory, 1, 5000, exp->output_folder, net);
    Callback* cb_data = callback_network_data_dumper_create(memory, exp->output_folder, net);
    Callback* cb_learning = callback_learning_history_create(memory, net);
    Callback* cb_accuracy = callback_accuracy_create(memory, exp->output_folder, net);

    experiment_set_network(exp, net);
    experiment_set_data_gen(exp, data_gen);
    experiment_set_epoch_count(exp, 10);

    experiment_add_callback(exp, cb_meta);
    experiment_add_callback(exp, cb_spikes);
    experiment_add_callback(exp, cb_weights);
    //experiment_add_callback(exp, cb_data);
    experiment_add_callback(exp, cb_learning);
    experiment_add_callback(exp, cb_accuracy);

    experiment_set_learning(exp, TRUE);

    experiment_run(exp);

    experiment_destroy(exp);

    // NOTE: currently I can alternate between input on and off, I get sync activity because a neuron on a layer gets more or less the same input Current as the others becuase
    // its fully connected

    return 0;
}