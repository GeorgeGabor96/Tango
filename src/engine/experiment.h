#ifndef __ENGINE_EXPERIMENT_H__
#define __ENGINE_EXPERIMENT_H__


typedef struct CallbackLink {
    Callback* callback;
    struct CallbackLink* next;
} CallbackLink;


typedef struct Experiment {
    Network* network;
    DataGen* data;
    CallbackLink* callbacks;
} Experiment;


internal Experiment* experiment_create(State* state, Network* network, DataGen* data);
internal void experiment_infer(Experiment* experiment, State* state, ThreadPool* pool);
internal void experiment_learn(Experiment* experiment, State* state, ThreadPool* pool);
internal void experiment_add_callback(Experiment* experiment, State* state, Callback* callback);

#endif // __ENGINE_EXPERIMENT_H__
