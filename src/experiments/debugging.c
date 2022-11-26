#include "tango.h"


Network* get_network(State* state) {
    Network* network = network_create(state, "Dummy network");
    
    // Add 5 layers, 2 inputs, 1 hidden and 2 outputs to test that all work
    NeuronCls* neuron_cls = neuron_cls_create_lif_refract(state,
                                                          "LIF refract cls", 5);
    SynapseCls* synapse_cls = synapse_cls_create(state,
                                                 "synapse cls",
                                                 SYNAPSE_VOLTAGE,
                                                 0, 1, 20, 1);
    
    Layer* layer_in_1 = layer_create(state, 
                                     "input 1", LAYER_DENSE, 100, neuron_cls);
    Layer* layer_in_2 = layer_create(state,
                                     "input 2", LAYER_DENSE, 400, neuron_cls);
    Layer* layer_hidden = layer_create(state,
                                       "hidden", LAYER_DENSE, 1000, neuron_cls);
    Layer* layer_out_1 = layer_create(state,
                                      "output 1", LAYER_DENSE, 100, neuron_cls);
    Layer* layer_out_2 = layer_create(state,
                                      "output 2", LAYER_DENSE, 10, neuron_cls);
    
    layer_link(state, layer_hidden, layer_in_1, synapse_cls, 1);
    layer_link(state, layer_hidden, layer_in_2, synapse_cls, 1);
    layer_link(state, layer_out_1, layer_hidden, synapse_cls, 1);
    layer_link(state, layer_out_2, layer_hidden, synapse_cls, 1);
    
    network_add_layer(network, layer_in_1, TRUE, FALSE);
    network_add_layer(network, layer_in_2, TRUE, FALSE);
    network_add_layer(network, layer_hidden, FALSE, FALSE);
    network_add_layer(network, layer_out_1, FALSE, TRUE);
    network_add_layer(network, layer_out_2, FALSE, TRUE);
    
    network_show(network);
    
    return network;
}



int main() {
    State* state = state_create();
    
    const char* output_folder = "D:\\repos\\Tango_threads";
    Network* network = get_network(state);
    DataGen* data = data_gen_create_random_spikes(state, 0.1f, 1, 10);
    Callback* callback = callback_dumper_create(state, output_folder, network);
    Simulator* sim = simulator_create(state, network, data, 4);
    simulator_add_callback(state, sim, callback);
    
    simulator_run(state, sim);
    
    timing_report(state->transient_storage, output_folder);
    
    state_destroy(state);
    
    memory_report();
    check(memory_is_empty() == TRUE, "We have memory leaks");
    
    error:
    
    
    return 0;
}