#include "simulator/callback.h"


internal Callback*
callback_dumper_create(const char* output_folder, Network* network) {
    String* output_folder_s = NULL;
    Callback* callback = NULL;
    u32 i = 0;
    Layer* layer = NULL;
    DumperLayerData* layer_data = NULL;
    
    check(output_folder != NULL, "output_folder is NULL");
    check(network != NULL, "network is NULL");
    
    output_folder_s = string_create(output_folder);
    check_memory(output_folder_s);
    
    callback = (Callback*)memory_malloc(sizeof(*callback),
                                        "callback_dumper_create dumper");
    check_memory(callback);
    
    callback->type = CALLBACK_NETWORK_DUMPER;
    
    callback->dumper.output_folder = output_folder_s;
    callback->dumper.sample_count = (u32)-1;
    callback->dumper.sample_duration = 0;
    callback->dumper.n_layers = network->n_layers;
    callback->dumper.layers_data = (DumperLayerData*)
        memory_malloc(sizeof(DumperLayerData) * network->n_layers,
                      "callback_dumper_create callback->dumper.data");
    check_memory(callback->dumper.layers_data);
    
    for (i = 0; i < network->n_layers; ++i) {
        layer = network->layers[i];
        layer_data = callback->dumper.layers_data + i;
        layer_data->n_neurons = layer->n_neurons;
        layer_data->name = layer->name;
    }
    bool result = os_folder_create_str(callback->dumper.output_folder);
    check(result == TRUE, "couldn't create folder %s", string_to_c_str(callback->dumper.output_folder));
    return callback;
    
    error:
    if (output_folder_s != NULL) string_destroy(output_folder_s);
    if (callback != NULL) {
        if (callback->dumper.layers_data != NULL) memory_free(callback->dumper.layers_data);
        memory_free(callback);
    }
    return NULL;
}


internal void
callback_dumper_destroy(Callback* callback) {
    check(callback != NULL, "callback is NULL");
    check(callback->type == CALLBACK_NETWORK_DUMPER,
          "callback->type should be %u (%s) not %u (%s)",
          CALLBACK_NETWORK_DUMPER,
          callback_type_get_c_str(CALLBACK_NETWORK_DUMPER),
          callback->type,
          callback_type_get_c_str(callback->type));
    
    // NOTE: free the info in the layer datas
    Dumper* dumper = &(callback->dumper);
    DumperLayerData* layer_data = NULL;
    u32 i = 0;
    
    for (i = 0; i < dumper->n_layers; ++i) {
        layer_data = dumper->layers_data + i;
        
        // NOTE: to be save in case begin sample fails
        if (layer_data->neurons_data == NULL) continue;
        memory_free(layer_data->neurons_data);
        
        layer_data->name = NULL;
        layer_data->n_neurons = 0;
        layer_data->neurons_data = NULL;
    }
    
    string_destroy(dumper->output_folder);
    memory_free(dumper->layers_data);
    memory_free(callback);
    memset(callback, 0, sizeof(*callback));
    
    error:
    return;
}


internal void
callback_dumper_begin_sample(Callback* callback,
                             Network* network,
                             u32 sample_duration) {
    Dumper* dumper = NULL;
    check(callback != NULL, "callback is NULL");
    check(callback->type == CALLBACK_NETWORK_DUMPER,
          "callback->type should be %u (%s) not %u (%s)",
          CALLBACK_NETWORK_DUMPER,
          callback_type_get_c_str(CALLBACK_NETWORK_DUMPER),
          callback->type,
          callback_type_get_c_str(callback->type));
    check(network != NULL, "network is NULL");
    check(sample_duration != 0, "sample_duration is 0");
    
    dumper = &(callback->dumper);
    bool should_alloc = sample_duration > dumper->sample_duration;
    u32 i = 0;
    
    DumperLayerData* layer_data = NULL;
    dumper->time = 0;
    dumper->sample_count += 1;
    dumper->sample_duration = sample_duration;
    
    // NOTE: Only alloc space if we need to, reuse if we can
    if (should_alloc != FALSE) {
        for (i = 0; i < dumper->n_layers; ++i) {
            layer_data = dumper->layers_data + i;
            if (layer_data->neurons_data != NULL) 
                memory_free(layer_data->neurons_data);
            
            layer_data->neurons_data = (DumperNeuronData*) memory_malloc(dumper->sample_duration * layer_data->n_neurons * sizeof(DumperNeuronData),
                                                                         "callback_dumper_begin_sample layer_data->steps_data");
            check_memory(layer_data->neurons_data);
        } 
    }
    return;
    error:
    
    // free all the data for the neurons
    if (dumper != NULL) {
        for (i = 0; i < dumper->n_layers; ++i) {
            layer_data = dumper->layers_data + i;
            if (layer_data->neurons_data == NULL) continue;
            memory_free(layer_data->neurons_data);
        }
    }
    
    return;
}



internal void
callback_dumper_update(Callback* callback, Network* network) {
    check(callback != NULL, "dumper is NULL");
    check(callback->type == CALLBACK_NETWORK_DUMPER,
          "callback->type should be %u (%s) not %u (%s)",
          CALLBACK_NETWORK_DUMPER,
          callback_type_get_c_str(CALLBACK_NETWORK_DUMPER),
          callback->type,
          callback_type_get_c_str(callback->type));
    check(network != NULL, "network is NULL");
    
    Dumper* dumper = &(callback->dumper);
    u32 layer_i = 0;
    u32 neuron_i = 0;
    u32 step = dumper->time;
    DumperLayerData* layer_data = NULL;
    DumperNeuronData* neuron_data = NULL;
    Neuron* neuron = NULL;
    Layer* layer = NULL;
    
    check(step < dumper->sample_duration,
          "step should be less than dumper->sample_duration");
    
    for (layer_i = 0; layer_i < network->n_layers; ++layer_i) {
        layer = network->layers[layer_i];
        layer_data = dumper->layers_data + layer_i;
        check(layer->n_neurons == layer_data->n_neurons, "layer->n_neurons != layer_data->n_neurons");
        
        // NOTE: Go to the current step
        neuron_data = layer_data->neurons_data + step * layer_data->n_neurons;
        
        for (neuron_i = 0; neuron_i < layer->n_neurons; ++neuron_i, ++neuron_data) {
            neuron = layer->neurons + neuron_i;
            
            neuron_data->voltage = neuron->voltage;
            neuron_data->spike = neuron->spike;
            neuron_data->psc = neuron->epsc + neuron->ipsc;
            neuron_data->epsc = neuron->epsc;
            neuron_data->ipsc = neuron->ipsc;
        }
    }
    
    ++(dumper->time); // GO to the next step
    
    error:
    return;
}


internal void
callback_dumper_end_sample(Callback* callback, Network* network) {
    check(callback != NULL, "dumper is NULL");
    check(callback->type == CALLBACK_NETWORK_DUMPER,
          "callback->type should be %u (%s) not %u (%s)",
          CALLBACK_NETWORK_DUMPER,
          callback_type_get_c_str(CALLBACK_NETWORK_DUMPER),
          callback->type,
          callback_type_get_c_str(callback->type));
    check(network != NULL, "network is NULL");
    
    FILE* fp = NULL;
    u32 i = 0;
    DumperLayerData* layer_data = NULL;
    Dumper* dumper = &(callback->dumper);
    char file_name[100] = { 0 };
    
    sprintf(file_name, "sample_%d.bin", dumper->sample_count);
    ++(dumper->sample_count);
    
    String* output_file = string_path_join_c_str(dumper->output_folder,
                                                 file_name,
                                                 TRUE); // need the first string for further calls
    log_info("Dumping in %s", string_to_c_str(output_file));
    
    fp = fopen(string_to_c_str(output_file), "wb");
    check(fp != NULL, "fp is NULL");
    string_destroy(output_file);
    
    fwrite(&(dumper->sample_duration), sizeof(u32), 1, fp);
    fwrite(&(dumper->n_layers), sizeof(u32), 1, fp);
    
    for (i = 0; i < dumper->n_layers; ++i) {
        layer_data = dumper->layers_data + i;
        
        fwrite(&(layer_data->name->length), sizeof(u32), 1, fp);
        fwrite(layer_data->name->data, sizeof(char), layer_data->name->length, fp);
        fwrite(&(layer_data->n_neurons), sizeof(u32), 1, fp);
        fwrite(layer_data->neurons_data, sizeof(DumperNeuronData),
               dumper->sample_duration * layer_data->n_neurons, fp);
    }
    
    fflush(fp);
    fclose(fp);
    
    error:
    return;
}