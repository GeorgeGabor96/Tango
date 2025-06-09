#include "tango.h"


int main() {
    Experiment* exp = experiment_create(0, 273407, "D:\\repos\\Tango\\outputs\\158_two_layer_net\\custom_learning");

    Memory* memory = exp->permanent_memory;

    DataGen* basic_experiment_data = data_gen_create_2_inputs(memory, exp->random, 0.05, 10);

    Network* net = network_create(memory);
    NeuronCls* n_cls = neuron_cls_create_if_refract(memory, string_create(memory, "if"), 10);

    Layer* in = layer_create(memory, string_create(memory, "in"), LAYER_DENSE, 2, n_cls);
    Layer* out = layer_create(memory, string_create(memory, "out"), LAYER_DENSE, 2, n_cls);

    f32 min_w = -1.0f;
    f32 max_w = 1.0f;
    SynapseCls* s_cls = synapse_cls_create(memory, string_create(memory, "s_cls"), SYNAPSE_CONDUCTANCE, 0.0f, SYNAPSE_DEFAULT_AMP, 5, 10);
    synapse_cls_add_learning_rule_rstdp_exponential(s_cls, min_w, max_w, 1, 1, 20);

    network_add_neuron_cls(net, n_cls, memory);
    network_add_synapse_cls(net, s_cls, memory);

    LayerLinkSynapseInitMeta* synapse_connect_info = create_layer_init_synapse_init_meta(memory, min_w, max_w, 1, 1);
    layer_link(out, in, s_cls, synapse_connect_info, memory);

    network_add_layer(net, in, TRUE, FALSE, memory);
    network_add_layer(net, out, FALSE, TRUE, memory);
    network_build(net, memory, exp->random);
    network_show(net);

    Callback* cb_meta = callback_meta_dumper_create(memory, exp->output_folder, net);
    Callback* cb_spikes = callback_spikes_dumper_create(memory, exp->output_folder, net);
    Callback* cb_weights = callback_weights_dumper_create(memory, 1, 10000, exp->output_folder, net);
    //Callback* cb_data = callback_network_data_dumper_create(memory, exp->output_folder, net);
    Callback* cb_stdp_learning_history = callback_learning_history_create(memory, net);
    Callback* cb_accuracy = callback_accuracy_create(memory, exp->output_folder, net);
    Callback* cb_learning_custom = callback_learning_custom_create(memory, net);

    experiment_set_network(exp, net);
    experiment_set_data_gen(exp, basic_experiment_data);

    experiment_add_callback(exp, cb_meta);
    experiment_add_callback(exp, cb_spikes);
    experiment_add_callback(exp, cb_weights);
    experiment_add_callback(exp, cb_learning_custom);
    experiment_add_callback(exp, cb_accuracy);

    experiment_set_epoch_count(exp, 3);
    experiment_set_learning(exp, TRUE);

    experiment_run(exp);

    experiment_destroy(exp);

    return 0;
}