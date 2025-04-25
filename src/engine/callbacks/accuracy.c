internal Callback*
callback_accuracy_create(Memory* memory, Network* network, u8 cooldown_value)
{
    Callback* callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    callback->type = CALLBACK_ACCURACY;
    callback->network = network;
    callback->accuracy.true_positives = 0;
    callback->accuracy.true_negatives = 0;
    callback->accuracy.false_positives = 0;
    callback->accuracy.false_negatives = 0;

    return callback;

    error:
    return NULL;
}


internal CALLBACK_BEGIN_SAMPLE(callback_accuracy_begin_sample)
{

}


internal CALLBACK_UPDATE(callback_accuracy_update)
{

}


internal CALLBACK_END_SAMPLE(callback_accuracy_end_sample)
{

}


internal CALLBACK_BEGIN_EXPERIMENT(callback_accuracy_begin_experiment)
{

}

internal CALLBACK_END_EXPERIMENT(callback_accuracy_end_experiment)
{

}