#ifndef __ENGINE_LAYER_H__
#define __ENGINE_LAYER_H__


typedef enum {
    LAYER_INVALID,
    LAYER_DENSE
} LayerType;


internal char* layer_type_get_c_str(LayerType type);


typedef struct LayerLink LayerLink;
typedef struct Layer Layer;


typedef struct LayerLinkSynapseInitMeta
{
    f32 min_w;
    f32 max_w;
    f32 connect_chance;
    f32 excitatory_chance;
} LayerLinkSynapseInitMeta;

internal LayerLinkSynapseInitMeta* create_layer_init_synapse_init_meta(Memory* memory, f32 min_w, f32 max_w, f32 connect_chance, f32 excitatory_chance);

// NOTE: could have multiple types of linking?
// NOTE: or multiple types of weight initializers?
// NOTE: fow now its too complitated maybe, lets add the simplest thing and see in the future if we need more
struct LayerLink {
    Layer* layer;
    SynapseCls* cls;
    LayerLinkSynapseInitMeta* synapse_meta;
    struct LayerLink* next;
};


struct Layer {
    LayerType type;
    String* name;
    NeuronCls* neuron_cls;

    LayerLink* inputs;
    LayerLink* outputs;
    u32 n_inputs;
    u32 n_outputs;

    Neuron* neurons;
    u32 n_neurons;

    b32 it_ran;
};


typedef enum {
    LAYER_TASK_STEP,
    LAYER_TASK_STEP_INJECT_CURRENT,
    LAYER_TASK_STEP_FORCE_SPIKE,
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
    u32 time;
    u32 neuron_start_i;
    u32 neuron_end_i;
} LayerTask;


// NOTE: The layer takes ownership of the name
internal Layer* layer_create(Memory* memory, String* name, LayerType type,
                             u32 n_neurons, NeuronCls* cls);

internal void layer_show(Layer* layer);
internal u32 layer_get_n_in_synapses(Layer* layer);

internal b32 layer_link(Layer* layer, Layer* input_layer, SynapseCls* cls, LayerLinkSynapseInitMeta* synapse_meta, Memory* memory);
internal u32 layer_link_synapses(Layer* layer, LayerLink* link, Synapse* synapses, u32 offset, Memory* memory, Random* random);
internal void layer_init_neurons(Layer* layer);

internal void layer_process_neurons(void* task);

internal void layer_step(Layer* layer, u32 time, Memory* memory, ThreadPool* pool);
internal void layer_step_inject_current(Layer* layer,
                                        u32 time, Currents* currents,
                                        Memory* memory, ThreadPool* pool);
internal void layer_step_force_spike(Layer* layer, u32 time, Spikes* spikes,
                                     Memory* memory, ThreadPool* pool);

internal void layer_clear(Layer* layer);

internal f32* layer_get_voltages(Memory* memory, Layer* layer);
internal f32* layer_get_pscs(Memory* memory, Layer* layer);
internal f32* layer_get_epscs(Memory* memory, Layer* layer);
internal f32* layer_get_ipscs(Memory* memory, Layer* layer);
internal b32* layer_get_spikes(Memory* memory, Layer* layer);

internal void layer_set_neuron_history(Layer* layer, u32 sample_duration, Memory* memory);

#endif // __ENGINE_LAYER_H__
