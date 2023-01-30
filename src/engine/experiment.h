#ifndef __ENGINE_EXPERIMENT_H__
#define __ENGINE_EXPERIMENT_H__

#define BLOCK_SIZE MB(10)


typedef struct CallbackLink {
    Callback* callback;  // can't we directly keep the callback not the pointer?
    struct CallbackLink* next;
} CallbackLink;


typedef struct Experiment {
    String* output_folder;

    Memory* permanent_memory;
    Memory* transient_memory;
    ThreadPool* pool;

    Network* network;

    DataGen* data;

    CallbackLink* callback_list;
    u32 n_callbacks;
} Experiment;


internal Experiment* experiment_create();
internal void experiment_destroy();
internal void experiment_infer(Experiment* exp);
internal void experiment_learn(Experiment* exp);
internal void experiment_timing_report(Experiment* exp);
internal void experiment_memory_report(Experiment* exp);

#endif // __ENGINE_EXPERIMENT_H__
