#include "tango.h"


Network* get_network(State* state) {
    f32 connect_chance = 0.25;

    Network* network = network_create(state, "Dummy network");

    NeuronCls* neuron_cls = neuron_cls_create_lif_refract(state,
                                                          "LIF refract cls", 5);
    SynapseCls* synapse_cls_exci = synapse_cls_create(state,
                                                      "AMPA",
                                                      SYNAPSE_VOLTAGE,
                                                      0.0f, 0.1f, 1, 10);
    SynapseCls* synapse_cls_inhi = synapse_cls_create(state,
                                                      "GABA_A",
                                                      SYNAPSE_VOLTAGE,
                                                      -90.0f, 0.1f, 6, 10);
    Layer* layer_in_exci = layer_create(state, "input_exci", LAYER_DENSE,
                                        90, neuron_cls);
    Layer* layer_in_inhi = layer_create(state, "input_inhi", LAYER_DENSE,
                                        10, neuron_cls);

    network_add_layer(state, network, layer_in_exci, TRUE, FALSE);
    network_add_layer(state, network, layer_in_inhi, TRUE, FALSE);

    Layer* last_layer_exci = layer_in_exci;
    Layer* last_layer_inhi = layer_in_inhi;

    for (u32 i = 1; i < 10; ++i) {
        char name_buffer[100] = { 0 };
        sprintf(name_buffer, "layer_%d_exci", i);
        Layer* cur_layer_exci = layer_create(state, name_buffer, LAYER_DENSE,
                                             90, neuron_cls);
        sprintf(name_buffer, "layer_%d_inhi", i);
        Layer* cur_layer_inhi = layer_create(state, name_buffer, LAYER_DENSE,
                                             10, neuron_cls);

        layer_link(state, cur_layer_exci, last_layer_exci,
                   synapse_cls_exci, 1, connect_chance);
        layer_link(state, cur_layer_exci, last_layer_inhi,
                   synapse_cls_inhi, 1, connect_chance);
        layer_link(state, cur_layer_inhi, last_layer_exci,
                   synapse_cls_exci, 1, connect_chance);
        layer_link(state, cur_layer_inhi, last_layer_inhi,
                   synapse_cls_inhi, 1, connect_chance);

        network_add_layer(state, network, cur_layer_exci, FALSE, FALSE);
        network_add_layer(state, network, cur_layer_inhi, FALSE, FALSE);

        last_layer_exci = cur_layer_exci;
        last_layer_inhi = cur_layer_inhi;
    }

    Layer* layer_out_exci = layer_create(state, "output_exci", LAYER_DENSE,
                                         100, neuron_cls);
    layer_link(state, layer_out_exci, last_layer_exci, synapse_cls_exci,
               1, connect_chance);
    layer_link(state, layer_out_exci, last_layer_inhi, synapse_cls_inhi,
               1, connect_chance);

    network_add_layer(state, network, layer_out_exci, FALSE, TRUE);

    network_build(state, network);
    network_show(network);

    return network;
}



int main() {
    State* state = state_create();
    random_init();

    const char* output_folder = "D:\\repos\\Tango_outputs\\synfire_chain";
    Network* network = get_network(state);
    DataGen* data = data_gen_create_spike_pulses(state, 2, 1000, 100, 20, 50, 0.1f, 0.01f);
    Callback* callback = callback_dumper_create(state, output_folder, network);

    ThreadPool* pool = thread_pool_create(4, layer_process_neurons, state->permanent_storage);

    Simulator* sim = simulator_create(state, network, data);
    simulator_add_callback(sim, state, callback);

    simulator_learn(sim, state, pool);
    thread_pool_stop(pool);

    timing_report(state->transient_storage, output_folder);

    state_destroy(state);

    memory_report();
    check(memory_is_empty() == TRUE, "We have memory leaks");

    error:

    return 0;
}
