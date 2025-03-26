#include "tango.h"


int main() {
    Experiment* exp = experiment_create(0, 723104, "D:\\repos\\Tango\\outputs\\144_basic_experiment");

    Memory* memory = exp->permanent_memory;

    DataGen* basic_experiment_data = data_gen_create_basic_experiment(memory, exp->random, 0.05, 100);

    Network* net = network_create(memory);
    NeuronCls* n_cls = neuron_cls_create_if_refract(memory, string_create(memory, "if"), 10);

    Layer* in = layer_create(memory, string_create(memory, "in"), LAYER_DENSE, 4, n_cls);
    Layer* hidden = layer_create(memory, string_create(memory, "hidden"), LAYER_DENSE, 4, n_cls);
    Layer* out = layer_create(memory, string_create(memory, "out"), LAYER_DENSE, 2, n_cls);

    SynapseCls* s_cls = synapse_cls_create_exci(memory, string_create(memory, "s_cls"), SYNAPSE_VOLTAGE, 10);
    synapse_cls_add_learning_rule_step(s_cls, -1.0f, 1.0f, 20, 0.1f, 20, -0.1f);

    network_add_neuron_cls(net, n_cls, memory);
    network_add_synapse_cls(net, s_cls, memory);

    layer_link(hidden, in, s_cls, -1.0f, 1.0f, 1.0f, memory);
    layer_link(out, hidden, s_cls, -1.0f, 1.0f, 1.0f, memory);

    network_add_layer(net, in, TRUE, FALSE, memory);
    network_add_layer(net, hidden, FALSE, FALSE, memory);
    network_add_layer(net, out, FALSE, TRUE, memory);
    network_build(net, memory, exp->random);
    network_show(net);

    Callback* cb_meta = callback_meta_dumper_create(memory, exp->output_folder, net);
    Callback* cb_spikes = callback_spikes_dumper_create(memory, exp->output_folder, net);
    Callback* cb_weights = callback_weights_dumper_create(memory, 1, 500, exp->output_folder, net);
    Callback* cb_data = callback_network_data_dumper_create(memory, exp->output_folder, net);
    Callback* cb_stdp_v1 = callback_stdp_v1_create(memory, net, 50);

    experiment_set_network(exp, net);
    experiment_set_data_gen(exp, basic_experiment_data);

    experiment_add_callback(exp, cb_meta);
    experiment_add_callback(exp, cb_spikes);
    experiment_add_callback(exp, cb_weights);
    experiment_add_callback(exp, cb_stdp_v1);

    experiment_set_learning(exp, TRUE);

    experiment_run(exp);

    experiment_destroy(exp);

    return 0;
}