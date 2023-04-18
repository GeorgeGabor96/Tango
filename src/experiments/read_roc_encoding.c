#include "tango.h"


int main() {
    Experiment* exp = experiment_create(0, 723104, "D:\\repos\\Tango_outputs\\exp\\mnist_roc_if_one_spike_new_sample");

    Memory* memory = exp->permanent_memory;

    DataGen* spike_train_data = data_gen_create_spike_train(memory, 500, "d:/datasets/MNIST/encoding/roc/img", "d:/datasets/MNIST/encoding/roc/img/samples.txt", 200, 10);
    //DataGen* spike_train_data = data_gen_create_spike_pulses(memory, exp->random, 2, 1000, 30, 100, 200, 0.05f, 0.001f);

    Network* net = network_create(memory);
    NeuronCls* n_cls = neuron_cls_create_if_one_spike(memory, string_create(memory, "if_one_spike"));
    Layer* in = layer_create(memory, string_create(memory, "in"), LAYER_DENSE, 784, n_cls);
    Layer* hidden = layer_create(memory, string_create(memory, "hidden"), LAYER_DENSE, 1000, n_cls);
    Layer* out = layer_create(memory, string_create(memory, "out"), LAYER_DENSE, 100, n_cls);

    SynapseCls* s_cls = synapse_cls_create(memory, string_create(memory, "s_cls"), SYNAPSE_VOLTAGE, 0.0f, 0.1f, 1, 10);
    synapse_cls_add_learning_rule_dan_poo(s_cls, -2.0f, 2.0f, 0.2f, -0.7f, 15);

    layer_link(hidden, in, s_cls, 0.2f, 0.05f, memory);
    layer_link(out, hidden, s_cls, 0.2f, 0.05f, memory);
    // TODO: add default synapse exci inhi

    network_add_layer(net, in, TRUE, FALSE, memory);
    network_add_layer(net, hidden, FALSE, FALSE, memory);
    network_add_layer(net, out, FALSE, TRUE, memory);

    network_build(net, memory, exp->random);

    network_show(net);

    Callback* cb_meta = callback_meta_dumper_create(
        memory, exp->output_folder, net);
    Callback* cb_spikes = callback_spikes_dumper_create(
        memory, exp->output_folder, net);

    experiment_set_network(exp, net);
    experiment_set_data_gen(exp, spike_train_data);
    experiment_add_callback(exp, cb_meta);
    experiment_add_callback(exp, cb_spikes);

    experiment_learn(exp);

    experiment_destroy(exp);

    return 0;
}