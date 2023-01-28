#ifndef __ENGINE_LAYER_H__
#define __ENGINE_LAYER_H__


typedef enum {
    LAYER_INVALID,
    LAYER_DENSE
} LayerType;


internal char* layer_type_get_c_str(LayerType type);


typedef struct LayerLink LayerLink;
typedef struct Layer Layer;


struct LayerLink {
    Layer* layer;
    SynapseCls* cls;
    f32 weight;
    f32 chance;
    struct LayerLink* next;
};


struct Layer {
    LayerType type;
    String* name;

    LayerLink* inputs;
    LayerLink* outputs;
    u32 n_inputs;
    u32 n_outputs;

    u32 n_neurons;
    u32 neuron_start_i;
    u32 neuron_end_i;

    bool it_ran;
};


typedef enum {
    LAYER_TASK_STEP,
    LAYER_TASK_STEP_INJECT_CURRENT,
    LAYER_TASK_STEP_FORCE_SPIKE,
    LAYER_TASK_LEARNING_STEP,
    LAYER_TASK_LEARNING_STEP_INJECT_CURRENT,
    LAYER_TASK_LEARNING_STEP_FORCE_SPIKE,
    LAYER_TASK_INVALID
} LayerTaskType;

typedef struct LayerStepData {
    LayerTaskType type;
    union {
        Currents* currents;
        Spikes* spikes;
    };
} LayerStepData;


typedef struct LayerTask {
    LayerStepData* data;
    Layer* layer;
    Neuron* neurons;
    Synapse* synapses;
    u32 time;
    u32 neuron_start_i;
    u32 neuron_end_i;
} LayerTask;


// NOTE: The layer takes ownership of the name
internal Layer* layer_create(State* state,
                             const char* name, LayerType type,
                             u32 n_neurons, NeuronCls* cls);

internal void layer_show(Layer* layer, Neuron* neurons);
internal u32 layer_get_n_in_synapses(Layer* layer, Neuron* neurons);

internal bool layer_link(State* state,
                         Layer* layer, Layer* input_layer,
                         SynapseCls* cls, f32 weight, f32 chance);

internal void layer_process_neurons(void* task);

internal void layer_step(Layer* layer, Neuron* neurons, Synapse* synapses,
                         u32 time, Memory* memory, ThreadPool* pool);
internal void layer_step_inject_current(Layer* layer, Neuron* neurons, Synapse* synapses,
                                        u32 time, Currents* currents,
                                        Memory* memory, ThreadPool* pool);
internal void layer_step_force_spike(Layer* layer, Neuron* neurons, Synapse* synapses,
                                     u32 time, Spikes* spikes,
                                     Memory* memory, ThreadPool* pool);

internal void layer_clear(Layer* layer, Neuron* neurons, Synapse* synapses);

internal f32* layer_get_voltages(Memory* memory, Layer* layer, Neuron* neurons);
internal f32* layer_get_pscs(Memory* memory, Layer* layer, Neuron* neurons);
internal f32* layer_get_epscs(Memory* memory, Layer* layer, Neuron* neurons);
internal f32* layer_get_ipscs(Memory* memory, Layer* layer, Neuron* neurons);
internal bool* layer_get_spikes(Memory* memory, Layer* layer, Neuron* neurons);

// LEARNING
internal void layer_learning_step(Layer* layer, Neuron* neurons, Synapse* synapses,
                                  u32 time, Memory* memory, ThreadPool* pool);
internal void layer_learning_step_inject_current(Layer* layer, Neuron* neurons, Synapse* synapses,
                                                 u32 time, Currents* currents,
                                                 Memory* memory, ThreadPool* pool);
internal void layer_learning_step_force_spike(Layer* layer, Neuron* neurons, Synapse* synapses,
                                              u32 time, Spikes* spikes,
                                              Memory* memory, ThreadPool* pool);
internal u32 layer_get_neuron_idx(Layer* layer, Neuron* neuron);

#endif // __ENGINE_LAYER_H__
