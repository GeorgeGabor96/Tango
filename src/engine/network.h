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

    bool is_built;
} Network;


internal Network* network_create(State* state, const char* name);

internal void network_show(Network* network);
internal bool network_build(State* state, Network* network);

internal void network_add_layer(State* state, Network* network, Layer* layer,
                                bool is_input, bool is_output);
internal void network_infer(Network* network, Inputs* inputs, u32 time,
                           Memory* memory, ThreadPool* pool);
internal void network_learn(Network* network, Inputs* inputs, u32 time,
                            Memory* memory, ThreadPool* pool);
internal void network_clear(Network* network);

#endif // __ENGINE_NETWORK_H__
