#ifndef __ENGINE_EXPERIMENT_H__
#define __ENGINE_EXPERIMENT_H__


typedef struct CallbackLink {
    Callback* callback;
    struct CallbackLink* next;
} CallbackLink;


typedef struct Experiment {
    String* output_folder;
    Memory* permanent_memory;
    Memory* transient_memory;
    ThreadPool* pool;
    Random* random;

    Network* network;
    DataGen* data;
    CallbackLink* callbacks;

    u32 n_epochs;
} Experiment;


internal Experiment* experiment_create(u32 n_workers, u32 seed, const char* output_folder);
internal void experiment_destroy(Experiment* experiment);
internal void experiment_set_learning(Experiment* experiment, b32 value);
internal void experiment_run(Experiment* experiment);
internal b32 experiment_add_callback(Experiment* experiment, Callback* callback);
internal b32 experiment_set_network(Experiment* experiment, Network* network);
internal b32 experiment_set_data_gen(Experiment* experiment, DataGen* data);
internal void experiment_set_epoch_count(Experiment* experiment, u32 n_epochs);

#endif // __ENGINE_EXPERIMENT_H__
