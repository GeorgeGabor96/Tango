#ifndef __ENGINE_EXPERIMENT_H__
#define __ENGINE_EXPERIMENT_H__


#define SIMULATOR_N_MAX_CALLBACKS 5u

typedef struct Experiment {
    Network* network;
    DataGen* data;
    Callback* callbacks[SIMULATOR_N_MAX_CALLBACKS];
    u32 n_callbacks;

    Neuron* neurons;
    Synapse* synapses;
    u32 n_neurons;
    u32 n_synapses;
} Experiment;


internal Experiment* experiment_create(State* state, Network* network, DataGen* data);
internal void experiment_infer(Experiment* experiment, State* state, ThreadPool* pool);
internal void experiment_learn(Experiment* experiment, State* state, ThreadPool* pool);
internal void experiment_add_callback(Experiment* experiment, State* state, Callback* callback);

#endif // __ENGINE_EXPERIMENT_H__
