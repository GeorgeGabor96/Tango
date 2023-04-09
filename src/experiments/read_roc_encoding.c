#include "tango.h"


int main() {
    Experiment* exp = experiment_create(0, 123, "D:\\repos\\Tango_outputs\\roc_encoding");

    Memory* memory = exp->permanent_memory;

    DataGen* spike_train_data = data_gen_create_spike_train(memory, 500, "d:/datasets/MNIST/encoding2/img_train", "d:/datasets/MNIST/encoding2/img_train/samples.txt");
    /*
    for (StringNode* node = roc_data->roc.first_file_name;
         node != NULL;
         node = node->next) {
        log_info("%s", string_get_c_str(node->name));
    }
    log_info("N_samples: %u", roc_data->n_samples);
    */

    Network* net = network_create(memory);
    NeuronCls* n_cls = neuron_cls_create_lif(memory, string_create(memory, "lif"));
    Layer* in = layer_create(memory, string_create(memory, "in"), LAYER_DENSE, 784, n_cls);
    Layer* out = layer_create(memory, string_create(memory, "out"), LAYER_DENSE, 10, n_cls);

    SynapseCls* s_cls = synapse_cls_create(memory, string_create(memory, "s_cls"), SYNAPSE_VOLTAGE, 0.0f, 0.1f, 1, 10);
    layer_link(out, in, s_cls, 1.0f, 1.0f, memory);
    // TODO: add default synapse exci inhi

    network_add_layer(net, in, TRUE, FALSE, memory);
    network_add_layer(net, out, FALSE, TRUE, memory);

    network_build(net, memory, random_create(memory, 123));

    network_show(net);

    Callback* cb = callback_dumper_create(memory, exp->output_folder, net);

    experiment_set_network(exp, net);
    experiment_set_data_gen(exp, spike_train_data);
    experiment_add_callback(exp, cb);

    experiment_infer(exp);

    experiment_destroy(exp);

    return 0;
}