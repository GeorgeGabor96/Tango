#include "tango.h"


int main() {
    Experiment* exp = experiment_create(4, 723104, "D:\\repos\\Tango_outputs\\weights\\stdp_exp_230");

    Memory* memory = exp->permanent_memory;

    DataGen* spike_train_data = data_gen_create_spike_train(memory, 300, "d:/datasets/MNIST/encoding_10_imgs", "d:/datasets/MNIST/encoding_10_imgs/samples.txt", 230, 500);
    //DataGen* spike_train_data = data_gen_create_spike_pulses(memory, exp->random, 31, 1000, 30, 100, 200, 0.05f, 0.001f);

    Network* net = network_create(memory);
    NeuronCls* n_cls = neuron_cls_create_if_one_spike(memory, string_create(memory, "if_one_spike"));
    Layer* in = layer_create(memory, string_create(memory, "in"), LAYER_DENSE, 784, n_cls);
    Layer* hidden = layer_create(memory, string_create(memory, "hidden"), LAYER_DENSE, 1000, n_cls);
    Layer* out = layer_create(memory, string_create(memory, "out"), LAYER_DENSE, 100, n_cls);

    SynapseCls* s_cls = synapse_cls_create(memory, string_create(memory, "s_cls"), SYNAPSE_VOLTAGE, 0.0f, 0.1f, 1, 10);
    synapse_cls_add_learning_rule_exponential(s_cls, 0.0f, 2.0f, 0.2f, -0.7f, 50);
    //synapse_cls_add_learning_rule_step(s_cls, 0.0f, 100.0f, 50, 0.5f, 50, -0.5f);

    layer_link(hidden, in, s_cls, 0.01f, 0.5f, 0.1f, memory);
    layer_link(out, hidden, s_cls, 0.01f, 0.5f, 0.1f, memory);
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
    Callback* cb_weights = callback_weights_dumper_create(
        memory, 10, 300, exp->output_folder, net);
    Callback* cb_rescale = callback_synaptic_rescale_create(
        memory, net, 20000);

    experiment_set_network(exp, net);
    experiment_set_data_gen(exp, spike_train_data);
    experiment_add_callback(exp, cb_meta);
    experiment_add_callback(exp, cb_spikes);
    experiment_add_callback(exp, cb_weights);
    experiment_add_callback(exp, cb_rescale);

    experiment_learn(exp);

    experiment_destroy(exp);

    return 0;
}