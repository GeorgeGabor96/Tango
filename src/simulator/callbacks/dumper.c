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
    
    callback = (Callback*)memory_malloc(sizeof(*callback), "callback_dumper_create dumper");
    check_memory(callback);
    
    callback->type = CALLBACK_NETWORK_DUMPER;
    
    callback->dumper.output_folder = output_folder_s;
    callback->dumper.sample_count = -1;
    callback->dumper.sample_duration = 0;
    callback->dumper.n_layers = network->n_layers;
    // NOTE: alloc the data for the layers
    callback->dumper.layers_data = (DumperLayerData*)memory_malloc(sizeof(DumperLayerData) * network->n_layers,
                                                                   "callback_dumper_create callback->dumper.data");
    check_memory(callback->dumper.layers_data);
    for (i = 0; i < network->n_layers; ++i) {
        layer = network->layers[i];
        layer_data = callback->dumper.layers_data + i;
        layer_data->n_neurons = layer->n_neurons;
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
    check(callback->type == CALLBACK_NETWORK_DUMPER, "callback->type is not CALLBACK_NETWORK_DUMPER");
    
    // NOTE: free the info in the layer datas
    Dumper* dumper = &(callback->dumper);
    DumperLayerData* layer_data = NULL;
    u32 layer_i = 0;
    u32 neuron_i = 0;
    
    for (layer_i = 0; layer_i < dumper->n_layers; ++layer_i) {
        layer_data = dumper->layers_data + layer_i;
        // NOTE: this may be freed already by the sample end
        if (layer_data->neurons_data == NULL) continue;
        memory_free(layer_data->neurons_data);
        
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
    check(callback != NULL, "callback is NULL");
    // TODO: add proper check for type
    check(callback->type == CALLBACK_NETWORK_DUMPER, "callback->type is %d",
          callback->type);
    check(network != NULL, "network is NULL");
    check(sample_duration != 0, "sample_duration is 0");
    
    Dumper* dumper = &(callback->dumper);
    dumper->time = 0;
    dumper->sample_count += 1;
    dumper->sample_duration = sample_duration;
    
    u32 layer_i = 0;
    u32 neuron_i = 0;
    DumperLayerData* layer_data = NULL;
    
    // TODO: We could do an optimization here, to only free if the current space is not big enough, to avoid more allocations than necessary
    
    for (layer_i = 0; layer_i < dumper->n_layers; ++layer_i) {
        layer_data = dumper->layers_data + layer_i;
        check(layer_data->neurons_data == NULL, "should had freed the steps");
        layer_data->neurons_data = (DumperNeuronData*) memory_malloc(dumper->sample_duration * layer_data->n_neurons * sizeof(DumperNeuronData),
                                                                     "callback_dumper_begin_sample layer_data->steps_data");
        check_memory(layer_data->neurons_data);
    } 
    return;
    error:
    
    // free all the data for the neurons
    for (u32 i = 0; i < layer_i; ++i) {
        layer_data = dumper->layers_data + layer_i;
        memory_free(layer_data->neurons_data);
    }
    
    return;
}



internal void
callback_dumper_update(Callback* callback, Network* network) {
    check(callback != NULL, "dumper is NULL");
    check(callback->type == CALLBACK_NETWORK_DUMPER, "Wrong callback type");
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
    check(callback->type == CALLBACK_NETWORK_DUMPER, "wrong callback type");
    check(network != NULL, "network is NULL");
    
    FILE* fp = NULL;
    u32 layer_i = 0;
    u32 neuron_i = 0;
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
    
    // NOTE: write the duration of sample and number of layers
    fwrite(&(dumper->sample_duration), sizeof(u32), 1, fp);
    fwrite(&(dumper->n_layers), sizeof(u32), 1, fp);
    
    // NOTE/TODO: We could just write in a big buffer and write the buffer to 
    // NOTE/TODO: a file bug, IO is usually buffered so shouldn't matter
    for (layer_i = 0; layer_i < dumper->n_layers; ++layer_i) {
        layer_data = dumper->layers_data + layer_i;
        
        // NOTE: for each layer write the number of neurons and
        // NOTE: for each neuron all the steps data
        fwrite(&(layer_data->n_neurons), sizeof(u32), 1, fp);
        fwrite(layer_data->neurons_data, sizeof(DumperNeuronData),
               dumper->sample_duration * layer_data->n_neurons, fp);
        
        memory_free(layer_data->neurons_data);
        layer_data->neurons_data = NULL;
    }
    
    fflush(fp);
    fclose(fp);
    
    error:
    return;
}