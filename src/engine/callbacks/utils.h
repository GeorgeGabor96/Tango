#ifndef __ENGINE_CALLBACK_UTILS_H__
#define __ENGINE_CALLBACK_UTILS_H__

internal b32 callback_utils_get_reward_first_spike(Network* network, DataSample* sample);
internal u32 callback_utils_get_winner_neuron(Network* net, DataSample* sample);

#endif