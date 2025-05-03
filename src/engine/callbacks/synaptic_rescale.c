internal inline void
_rescale_weights(Callback* callback) {
    SynapticRescale* data = &(callback->synaptic_rescale);
    f32 total_weight = 0.0f;
    u32 i = 0;
    Network* network = callback->network;
    Synapse* synapses = network->synapses;

    for (i = 0; i < network->n_synapses; i++) {
        total_weight += synapses[i].weight;
    }

    f32 factor = data->neurotransmitter_quantity / total_weight;

    for (i = 0; i < network->n_synapses; i++) {
        synapses[i].weight *= factor;
    }

#ifdef DEBUG_SYNAPTIC_SCALING
    f32 new_total_weight = 0.0f;
    for (i = 0; i < network->n_synapses; i++) {
        new_total_weight += synapses[i].weight;
    }
    log_info("[SYNAPTIC SCALING] total weight %f, factor %f, desired %f, new total weight %f",
             total_weight, factor, data->neurotransmitter_quantity, new_total_weight);

#endif
}


internal Callback*
callback_synaptic_rescale_create(
    Memory* memory,
    Network* network,
    f32 neurotransmitter_quantity)
{
    check(memory != NULL, "memory is NULL");
    check(network != NULL, "network is NULL");
    check(neurotransmitter_quantity > 0.0f, "neurotransmitter_quantity should be > 0");

    Callback* callback = (Callback*) memory_push(memory, sizeof(*callback));
    check_memory(callback);

    callback->type = CALLBACK_SYNAPTIC_RESCALE;
    callback->network = network;
    callback->synaptic_rescale.neurotransmitter_quantity = neurotransmitter_quantity;

    _rescale_weights(callback);

    return callback;
    error:
    return NULL;
}


internal CALLBACK_BEGIN_SAMPLE(callback_synaptic_rescale_begin_sample)
{

}


internal CALLBACK_UPDATE(callback_synaptic_rescale_update)
{

}


internal CALLBACK_END_SAMPLE(callback_synaptic_rescale_end_sample)
{
    _rescale_weights(callback);
}


internal CALLBACK_BEGIN_EPOCH(callback_synaptic_rescale_begin_epoch)
{

}


internal CALLBACK_END_EPOCH(callback_synaptic_rescale_end_epoch)
{

}


internal CALLBACK_BEGIN_EXPERIMENT(callback_synaptic_rescale_begin_experiment)
{

}


internal CALLBACK_END_EXPERIMENT(callback_synaptic_rescale_end_experiment)
{

}