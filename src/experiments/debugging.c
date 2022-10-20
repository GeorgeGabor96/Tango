#include "utils/memory.c"
#include "containers/string.c"
#include "containers/array.c"
#include "simulator/synapse.c"
#include "simulator/neuron.c"
#include "simulator/layer.c"
#include "simulator/network.c"
#include "simulator/data_gen.c"
#include "simulator/callback.c"
#include "simulator/simulator.c"



Network* get_network() {
    Network* network = network_create("Dummy network");
    
    // Add 5 layers, 2 inputs, 1 hidden and 2 outputs to test that all work
    NeuronCls* neuron_cls = neuron_cls_create_lif_refract("LIF refract cls", 5);
    SynapseCls* synapse_cls = synapse_cls_create("synapse cls", SYNAPSE_VOLTAGE,
                                                 0, 1, 20, 1);
    
    Layer* layer_in_1 = layer_create("input 1", LAYER_DENSE, 100, neuron_cls);
    Layer* layer_in_2 = layer_create("input 2", LAYER_DENSE, 400, neuron_cls);
    Layer* layer_hidden = layer_create("hidden", LAYER_DENSE, 1000, neuron_cls);
    Layer* layer_out_1 = layer_create("output 1", LAYER_DENSE, 100, neuron_cls);
    Layer* layer_out_2 = layer_create("output 2", LAYER_DENSE, 10, neuron_cls);
    
    layer_link(layer_hidden, layer_in_1, synapse_cls, 1);
    layer_link(layer_hidden, layer_in_2, synapse_cls, 1);
    layer_link(layer_out_1, layer_hidden, synapse_cls, 1);
    layer_link(layer_out_2, layer_hidden, synapse_cls, 1);
    
    network_add_layer(network, layer_in_1, TRUE, FALSE);
    network_add_layer(network, layer_in_2, TRUE, FALSE);
    network_add_layer(network, layer_hidden, FALSE, FALSE);
    network_add_layer(network, layer_out_1, FALSE, TRUE);
    network_add_layer(network, layer_out_2, FALSE, TRUE);
    
    network_show(network);
    
    return network;
}



int main() {
    Network* network = get_network();
    
    network_destroy(network);
    return 0;
}