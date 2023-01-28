#ifndef __ENGINE_NETWORK_H__
#define __ENGINE_NETWORK_H__


#define NETWORK_N_MAX_LAYERS 32u

typedef struct NetworkLayerLink {
    Layer* layer;
    struct NetworkLayerLink* next;
} NetworkLayerLink;

typedef struct Network {
    String* name;

    NetworkLayerLink* layers;
    NetworkLayerLink* inputs;
    NetworkLayerLink* outputs;

    u32 n_layers;
    u32 n_in_layers;
    u32 n_out_layers;

    Neuron* neurons;
    Synapse* synapses;
    u32 n_neurons;
    u32 n_synapses;

} Network;


internal Network* network_create(State* state, const char* name);

internal void network_show(Network* network);
internal void network_build(Network* network);

internal void network_add_layer(Network* network, Layer* layer,
                                bool is_input, bool is_output);
internal void network_infer(Network* network, Inputs* inputs, u32 time,
                           Memory* memory, ThreadPool* pool);
internal void network_learn(Network* network, Inputs* inputs, u32 time,
                            Memory* memory, ThreadPool* pool);
internal void network_clear(Network* network);

internal f32* network_get_layer_voltages(State* state, Network* network, u32 i);
internal bool* network_get_layer_spikes(State* state, Network* network, u32 i);

internal u32 network_get_layer_idx(Network* net, Layer* layer);

#endif // __ENGINE_NETWORK_H__
