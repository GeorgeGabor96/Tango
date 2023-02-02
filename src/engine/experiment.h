#ifndef __ENGINE_EXPERIMENT_H__
#define __ENGINE_EXPERIMENT_H__


typedef struct CallbackLink {
    Callback* callback;
    struct CallbackLink* next;
} CallbackLink;


typedef struct Experiment {
    Memory* permanent_memory;
    Memory* transient_memory;
    ThreadPool* pool;

    Network* network;
    DataGen* data;
    CallbackLink* callbacks;
} Experiment;


internal Experiment* experiment_create(u32 n_workers);
internal void experiment_infer(Experiment* experiment);
internal void experiment_learn(Experiment* experiment);
internal void experiment_add_callback(Experiment* experiment, Callback* callback);
internal void experiment_set_network(Experiment* experiment, Network* network);
internal void experiment_set_data_gen(Experiment* experiment, DataGen* data);

#endif // __ENGINE_EXPERIMENT_H__