#include "simulator/callback.h"


internal Callback*
callback_dumper_create(State* state, const char* output_folder, Network* network) {
    String* output_folder_s = NULL;
    Callback* callback = NULL;
    u32 i = 0;
    Layer* layer = NULL;
    DumperLayerData* layer_data = NULL;
    
    check(state != NULL, "state is NULL");
    check(output_folder != NULL, "output_folder is NULL");
    check(network != NULL, "network is NULL");
    
    output_folder_s = string_create(state->permanent_storage, output_folder);
    check_memory(output_folder_s);
    
    callback = (Callback*) memory_arena_push(state->permanent_storage, sizeof(*callback));
    check_memory(callback);
    
    callback->type = CALLBACK_NETWORK_DUMPER;
    
    callback->dumper.output_folder = output_folder_s;
    callback->dumper.sample_count = (u32)-1;
    callback->dumper.sample_duration = 0;
    callback->dumper.n_layers = network->n_layers;
    callback->dumper.layers_data = (DumperLayerData*) memory_arena_push(state->permanent_storage, sizeof(DumperLayerData) * network->n_layers);
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
    
    return NULL;
}


internal void
callback_dumper_begin_sample(State* state,
                             Callback* callback,
                             Network* network,
                             u32 sample_duration) {
    Dumper* dumper = NULL;
    check(state != NULL, "state is NULL");
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
    u32 i = 0;
    
    DumperLayerData* layer_data = NULL;
    dumper->time = 0;
    dumper->sample_count += 1;
    dumper->sample_duration = sample_duration;
    
    for (i = 0; i < dumper->n_layers; ++i) {
        layer_data = dumper->layers_data + i;
        layer_data->neurons_data = (DumperNeuronData*) memory_arena_push(state->transient_storage, dumper->sample_duration * layer_data->n_neurons * sizeof(DumperNeuronData));
        check_memory(layer_data->neurons_data);
    } 
    
    error:
    return;
}


internal void
callback_dumper_update(State* state, Callback* callback, Network* network) {
    check(state != NULL, "state is NULL");
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
callback_dumper_end_sample(State* state, Callback* callback, Network* network) {
    check(state != NULL, "state is NULL");
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
    
    String* output_file = string_path_join_c_str(state->transient_storage,
                                                 dumper->output_folder,
                                                 file_name);
    log_info("Dumping in %s", string_to_c_str(output_file));
    
    fp = fopen(string_to_c_str(output_file), "wb");
    check(fp != NULL, "fp is NULL");
    
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
