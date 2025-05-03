internal Callback*
callback_accuracy_create(Memory* memory, Network* network)
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
    // how to know if network was correct
}


internal CALLBACK_BEGIN_EPOCH(callback_accuracy_begin_epoch)
{
    callback->accuracy.true_positives = 0;
    callback->accuracy.true_negatives = 0;
    callback->accuracy.false_positives = 0;
    callback->accuracy.false_negatives = 0;
}

internal CALLBACK_END_EPOCH(callback_accuracy_end_epoch)
{
    Accuracy* accuracyData = &callback->accuracy;
    f32 accuracy = (accuracyData->true_positives + accuracyData->true_negatives) /
        (accuracyData->true_positives + accuracyData->true_negatives + accuracyData->false_positives + accuracyData->false_negatives + 0.00001);
    log_info("\n--------------------------\nEPOCH %u Accurary: %.2f\n--------------------------", epoch_i, accuracy);
}