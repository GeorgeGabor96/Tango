#ifndef __TANGO_H__
#define __TANGO_H__


#include "utils/utils.h"
#include "engine/engine.h"


void* tango_create(u32 n_workers, u32 seed, const char* output_folder);
void tango_destroy(void* instance);

b32 tango_create_neuron_cls_lif_refract(void* instance,
                                        const char* name, u32 refract_time);

b32 tango_create_synapse_cls(void* instance,
                             const char* name, SynapseType type,
                             f32 rev_potential, f32 amp,
                             f32 tau_ms, u32 delay);

b32 tango_create_layer(void* instance,
                       const char* name, LayerType type, u32 n_neurons,
                       const char* neuron_cls_name, b32 is_input, b32 is_output);

b32 tango_link_layers(void* instance,
                      const char* layer_name, const char* in_layer_name,
                      const char* synapse_cls_name, f32 synapse_weight,
                      f32 connect_chance);

b32 tango_build_network(void* instance);
void tango_show_network(void* instance);


b32 tango_create_data_constant_current(void* instance);
b32 tango_create_data_random_spikes(void* instance);
b32 tango_create_data_spike_pulses(void* instance,
                                   u32 n_samples, u32 sample_duration,
                                   u32 first_pulse_time, u32 pulse_duration,
                                   u32 between_pulses_duration,
                                   f32 pulse_spike_chance, f32 between_pulses_spike_chance);

b32 tango_create_callback_dumper(void* instance);

void tango_learn(void* instance);
void tango_infer(void* instance);


#include "tango.c"


#endif // __TANGO_H__
