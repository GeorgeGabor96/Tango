#include "tango.h"


Network* get_network(Experiment* exp) {
    f32 connect_chance = 0.25;
    Memory* memory = exp->permanent_memory;

    Network* network = network_create(memory, "Dummy network");

    NeuronCls* neuron_cls = neuron_cls_create_lif_refract(memory,
                                                          "LIF refract cls", 5);
    SynapseCls* synapse_cls_exci = synapse_cls_create(memory,
                                                      "AMPA",
                                                      SYNAPSE_VOLTAGE,
                                                      0.0f, 0.1f, 1, 10);
    SynapseCls* synapse_cls_inhi = synapse_cls_create(memory,
                                                      "GABA_A",
                                                      SYNAPSE_VOLTAGE,
                                                      -90.0f, 0.1f, 6, 10);
    Layer* layer_in_exci = layer_create(memory, "input_exci", LAYER_DENSE,
                                        90, neuron_cls);
    Layer* layer_in_inhi = layer_create(memory, "input_inhi", LAYER_DENSE,
                                        10, neuron_cls);

    network_add_layer(network, layer_in_exci, TRUE, FALSE, memory);
    network_add_layer(network, layer_in_inhi, TRUE, FALSE, memory);

    Layer* last_layer_exci = layer_in_exci;
    Layer* last_layer_inhi = layer_in_inhi;

    for (u32 i = 1; i < 10; ++i) {
        char name_buffer[100] = { 0 };
        sprintf(name_buffer, "layer_%d_exci", i);
        Layer* cur_layer_exci = layer_create(memory, name_buffer, LAYER_DENSE,
                                             90, neuron_cls);
        sprintf(name_buffer, "layer_%d_inhi", i);
        Layer* cur_layer_inhi = layer_create(memory, name_buffer, LAYER_DENSE,
                                             10, neuron_cls);

        layer_link(cur_layer_exci, last_layer_exci,
                   synapse_cls_exci, 1, connect_chance, memory);
        layer_link(cur_layer_exci, last_layer_inhi,
                   synapse_cls_inhi, 1, connect_chance, memory);
        layer_link(cur_layer_inhi, last_layer_exci,
                   synapse_cls_exci, 1, connect_chance, memory);
        layer_link(cur_layer_inhi, last_layer_inhi,
                   synapse_cls_inhi, 1, connect_chance, memory);

        network_add_layer(network, cur_layer_exci, FALSE, FALSE, memory);
        network_add_layer(network, cur_layer_inhi, FALSE, FALSE, memory);

        last_layer_exci = cur_layer_exci;
        last_layer_inhi = cur_layer_inhi;
    }

    Layer* layer_out_exci = layer_create(memory, "output_exci", LAYER_DENSE,
                                         100, neuron_cls);
    layer_link(layer_out_exci, last_layer_exci, synapse_cls_exci,
               1, connect_chance, memory);
    layer_link(layer_out_exci, last_layer_inhi, synapse_cls_inhi,
               1, connect_chance, memory);

    network_add_layer(network, layer_out_exci, FALSE, TRUE, memory);

    network_build(network, memory);
    network_show(network);

    return network;
}



int main() {
    random_init();

    Experiment* exp = experiment_create(4);
    Network* network = get_network(exp);

    const char* output_folder = "D:\\repos\\Tango_outputs\\synfire_chain";
    DataGen* data = data_gen_create_spike_pulses(exp->permanent_memory, 2, 1000, 100, 20, 50, 0.1f, 0.01f);
    Callback* callback = callback_dumper_create(exp->permanent_memory, output_folder, network);

    experiment_set_network(exp, network);
    experiment_set_data_gen(exp, data);
    experiment_add_callback(exp, callback);

    experiment_learn(exp);

    timing_report(exp->transient_memory, output_folder);

    experiment_destroy(exp);

    memory_report();
    check(memory_is_empty() == TRUE, "We have memory leaks");

    error:

    return 0;
}
