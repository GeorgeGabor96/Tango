/* date = October 2nd 2022 9:18 pm */

#ifndef LAYER_H
#define LAYER_H

#include "common.h"
#include "utils/memory.h"
#include "containers/string.h"
#include "containers/array.h"
#include "simulator/neuron.h"


typedef enum {
    LAYER_INVALID,
    LAYER_DENSE
} LayerType;


internal char* layer_type_get_c_str(LayerType type);


#define LAYER_N_MAX_IN_LAYERS 5

typedef struct Layer {
    LayerType type;
    String* name;
    
    Neuron* neurons;
    u32 n_neurons;
    bool it_ran;
    
    struct Layer* in_layers[LAYER_N_MAX_IN_LAYERS];
    u32 n_in_layers;
} Layer;


typedef enum {
    LAYER_TASK_STEP,
    LAYER_TASK_STEP_INJECT_CURRENT,
    LAYER_TASK_STEP_FORCE_SPIKE,
    LAYER_TASK_LEARNING_STEP,
    LAYER_TASK_LEARNING_STEP_INJECT_CURRENT,
    LAYER_TASK_LEARNING_STEP_FORCE_SPIKE,
    LAYER_TASK_INVALID
} LayerTaskType;


typedef struct LayerTask {
    NetworkInput* input;
    Layer* layer;
    u32 time;
    u32 neuron_start_i;
    u32 neuron_end_i;
} LayerTask;


// NOTE: The layer takes ownership of the name
internal Layer* layer_create(State* state, 
                             const char* name, LayerType type,
                             u32 n_neurons, NeuronCls* cls);

internal void layer_show(Layer* layer);
internal u32 layer_get_n_in_synapses(Layer* layer);

internal bool layer_link(State* state, 
                         Layer* layer, Layer* input_layer,
                         SynapseCls* cls, f32 weight, f32 chance); 

internal void layer_process_neurons(void* task); 

internal void layer_step(Layer* layer, u32 time, MemoryArena* storage, ThreadPool* pool);
internal void layer_step_inject_current(Layer* layer, u32 time, Currents* currents,
                                        MemoryArena* storage, ThreadPool* pool);
internal void layer_step_force_spike(Layer* layer, u32 time, Spikes* spikes,
                                     MemoryArena* storage, ThreadPool* pool);

internal void layer_clear(Layer* layer);

internal f32* layer_get_voltages(MemoryArena* arena, Layer* layer);
internal f32* layer_get_pscs(MemoryArena* arena, Layer* layer);
internal f32* layer_get_epscs(MemoryArena* arena, Layer* layer);
internal f32* layer_get_ipscs(MemoryArena* arena, Layer* layer);
internal bool* layer_get_spikes(MemoryArena* arena, Layer* layer);

// LEARNING
internal void layer_learning_step(Layer* layer, u32 time, MemoryArena* storage, ThreadPool* pool);
internal void layer_learning_step_inject_current(Layer* layer, u32 time, Currents* currents,
                                                 MemoryArena* storage, ThreadPool* pool);
internal void layer_learning_step_force_spike(Layer* layer, u32 time, Spikes* spikes,
                                              MemoryArena* storage, ThreadPool* pool);

#endif //LAYER_H
