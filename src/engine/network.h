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

typedef struct Network {
    String* name;

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


internal Network* network_create(Memory* memory, const char* name);

internal void network_show(Network* network);
internal b32 network_build(Network* network, Memory* memory);

internal void network_add_layer(Network* network, Layer* layer,
                                b32 is_input, b32 is_output,
                                Memory* memory);
internal void network_infer(Network* network, Inputs* inputs, u32 time,
                           Memory* memory, ThreadPool* pool);
internal void network_learn(Network* network, Inputs* inputs, u32 time,
                            Memory* memory, ThreadPool* pool);
internal void network_clear(Network* network);

#endif // __ENGINE_NETWORK_H__
