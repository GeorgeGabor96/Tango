#ifndef __ENGINE_NETWORK_H__
#define __ENGINE_NETWORK_H__


typedef struct NetworkLayerLink {
    Layer* layer;
    struct NetworkLayerLink* next;
} NetworkLayerLink;

typedef struct NetworkLayerList {
    NetworkLayerLink* first;
    NetworkLayerLink* last;
} NetworkLayerList;

typedef struct NetworkNeuronClsLink {
    NeuronCls* cls;
    struct NetworkNeuronClsLink* next;
} NetworkNeuronClsLink;

typedef struct NetworkSynapseClsLink {
    SynapseCls* cls;
    struct NetworkSynapseClsLink* next;
} NetworkSynapseClsLink;

typedef struct Network {
    NetworkNeuronClsLink* neuron_classes;
    NetworkSynapseClsLink* synapse_classes;
    u32 n_neuron_cls;
    u32 n_synapse_cls;

    NetworkLayerList layers;
    NetworkLayerList in_layers;
    NetworkLayerList out_layers;

    u32 n_layers;
    u32 n_in_layers;
    u32 n_out_layers;

    Neuron* neurons;
    Synapse* synapses;
    u32 n_neurons;
    u32 n_synapses;

    b32 is_built;
} Network;


internal Network* network_create(Memory* memory);

internal void network_show(Network* network);
internal b32 network_build(Network* network, Memory* memory, Random* random);

internal void network_add_neuron_cls(Network* network, NeuronCls* cls,
                                     Memory* memory);
internal void network_add_synapse_cls(Network* network, SynapseCls* cls,
                                      Memory* memory);
internal void network_add_layer(Network* network, Layer* layer,
                                b32 is_input, b32 is_output,
                                Memory* memory);

internal NeuronCls* network_get_neuron_cls(Network* network,
                                           String* neuron_cls_name);
internal SynapseCls* network_get_synapse_cls(Network* network,
                                             String* synapse_cls_name);
internal Layer* network_get_layer(Network* network, String* layer_name);

internal void network_set_learning(Network* network, b32 value);
internal void network_step(Network* network, Inputs* inputs, u32 time,
                           Memory* memory, ThreadPool* pool);
internal void network_clear(Network* network);
internal void network_set_neuron_history(Network* network, u32 sample_duration, Memory* memory);

#endif // __ENGINE_NETWORK_H__
