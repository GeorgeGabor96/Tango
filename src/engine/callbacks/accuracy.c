internal Callback*
callback_accuracy_create(Memory* memory, String* output_folder, Network* network)
{
    check(memory != NULL, "memory is NULL");
    check(output_folder != NULL, "output_folder is NULL");
    check(network != NULL, "network is NULL");

    b32 result = os_folder_create_str(output_folder);
    check(result == TRUE, "couldn't create folder %s", string_get_c_str(output_folder));

    char file_name[100] = { 0 };
    sprintf(file_name, "accuracy.txt");
    String* file_name_s = string_create(memory, file_name);
    check_memory(file_name_s);

    String* file_path = string_path_join(memory, output_folder, file_name_s);
    check(file_path != NULL, "Couldn't build path for sample file %s", file_name);

    FILE* fp = fopen(string_get_c_str(file_path), "w");
    check(fp != NULL, "Could not open sample file %s", string_get_c_str(file_path));

    Callback* callback = (Callback*)memory_push(memory, sizeof(*callback));
    check_memory(callback);

    callback->type = CALLBACK_ACCURACY;
    callback->network = network;
    callback->accuracy.hits = 0;
    callback->accuracy.total = 0;
    callback->accuracy.fp = fp;

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
    // compare it to what was expected and update the metrics
    Accuracy* accuracyData = &callback->accuracy;
    if (reward == TRUE)
    {
        accuracyData->hits++;
    }
    accuracyData->total++;

    error:
    return;
}


internal CALLBACK_BEGIN_EPOCH(callback_accuracy_begin_epoch)
{
    callback->accuracy.hits = 0;
    callback->accuracy.total = 0;
}


internal CALLBACK_END_EPOCH(callback_accuracy_end_epoch)
{
    Accuracy* accuracyData = &callback->accuracy;
    f32 accuracy = accuracyData->hits / (accuracyData->total + 0.00001);
    log_info("\n--------------------------\nEPOCH %u Accurary: %.2f\n--------------------------", epoch_i, accuracy);

    fprintf(accuracyData->fp, "%.02f\n", accuracy);
}


internal CALLBACK_BEGIN_EXPERIMENT(callback_accuracy_begin_experiment)
{

}


internal CALLBACK_END_EXPERIMENT(callback_accuracy_end_experiment)
{
    Accuracy* accuracy = &callback->accuracy;
    fflush(accuracy->fp);
    fclose(accuracy->fp);
}