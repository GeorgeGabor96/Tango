typedef struct DumpLayerData {
    u32 n_steps;
    u32 n_neurons; // Here isn't enough?
    DumperNeuronDataP* steps;
} DumpLayerData;


// TODO: Can't I use this directly in the neuron class???
typedef struct DumperNeuronData {
    f32 voltage;
    bool spike;
    f32 psc;
    f32 epsc;
    f32 ipsc;
} DumperNeuronData, *DumperNeuronDataP;


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
    
    Dumper dumper = callback->Dumper;
    dumper.time = 0;
    dumper.sample_count += 1;
    dumper.duration = sample_duration;
    
    
    // NOTE: for each layer alloc sample_duration steps
    // NOTE: in each steps save all the neurons info for that layer
    for (u32 layer_i = 0; layer_i < dumper.n_layers; ++layer_i) {
        DumpLayerData* layer_data = dumper.data + layer_i;
        check(layer_data->steps == NULL, "should had freed the steps");
        layer_data->steps = (DumperNeuronDataP*) memory_malloc(sizeof(DumperNeuronDataP) * layer_data->n_neurons,
                                                               "callback_dumper_begin_sample layer_data->steps");
        check_memory(layer_data->steps);
        
        for (u32 time = 0; time < layer_data->n_steps; ++time) {
            layer_data->steps[time] = (DumperNeuronData*) memory_malloc(sizeof(DumperNeuronData) * layer_data->n_neurons,
                                                                        "callback_dumper_begin_sample layer_data->steps[time]");
            check_memory(layer_data->steps[time]);
        }
    } 
    
    error:
    // TODO: for memory failure frees
    return;
}



internal void
callback_dumper_update(Callback* callback, Network* network) {
    check(dumper != NULL, "dumper is NULL");
    check(network != NULL, "network is NULL");
    
    Dumper dumper = callback->Dumper;
    
    for (u32 layer_i = 0; layer_i < network->n_layers; ++layer_i) {
        Layer* layer = network->layers[layer_i];
        DumpLayerData* layer_data = dumper.data + layer_i;
        DumpNeuonData* step = layer_data->steps[dumper.time];
        
        for (u32 neuron_i = 0; neuron_i < layer->n_neurons; ++neuron_i) {
            DumperNeuronData* neurons_data = step + neuron_i;
            Neuron* neuron = layer->neurons + neuron_i;
            
            neurons_data.voltage = neuron.voltage;
            neurons_data.spike = neuron.spike;
            neurons_data.psc = neuron.psc;
            neurons_data.epsc = neuron.epsc;
            neurons_data.ipsc = neuron.ipsc;
        }
        
    }
    
    ++(dumper->dumper.time);
    
    error;
    return;
}


internal void
callback_dumper_end_sample(Callback* callback, Network* network) {
    check(dumper != NULL, "dumper is NULL");
    check(network != NULL, "network is NULL");
    
    Dumper dumper = callback->Dumper;
    char file_name[100] = { 0 };
    sprintf(file_name, "sample_%d.bin", dumper.sample_count);
    
    String* output_file = string_path_join_c_str(callback->dumper.output_folder,
                                                 file_name);
    log_info("Dumping in %s", string_to_c_str(output_file));
    
    FILE* fp = fopen(string_to_c_str(output_file), "wb");
    check(fp != NULL, "pf is NULL");
    
    // NOTE: write the duration of sample and number of layers
    fwrite(&(dumper.duration), sizeof(u32), 1, fp);
    fwrite(&(dumper.n_layers), sizeof(u32), 1, fp);
    
    for (u32 layer_i = 0; layer_i < dumper.n_layers; ++layer_i) {
        DumpLayerData* layer_data = dumper.data + layer_i;
        
        // NOTE: for each layer write the number of neurons and
        // NOTE: all the steps data
        fwrite(&(layer_data->n_neurons), sizeof(u32), 1, fp);
        
        for (u32 time = 0; time < dumper.duration; ++time) {
            DumperNeuronData* step = layer_data->steps[time];
            
            fwrite(step, sizeof(DumpNeuronData), layer_data->n_neurons, pf);
            
            // free the data for the step
            memory_free(step);
        }
        
        // free the data for the steps
        memory_free(layer_Data->steps);
    }
    
    fflush(fp);
    fclose(fp);
    
    error:
    return;
}

// TODO: below
internal Callback*
callback_network_dumper_create(const char* output_folder, Network* network) {
    String* output_folder_s = NULL;
    Callback* callback = NULL;
    
    check(output_folder != NULL, "output_folder is NULL");
    
    output_folder_s = string_create(output_folder);
    check_memory(output_folder_s);
    
    callback = (Callback*)memory_malloc(sizeof(*callback), "callback_network_dumper_create");
    check_memory(callback);
    
    callback->type = CALLBACK_NETWORK_DUMPER;
    callback->dumper.output_folder = output_folder_s;
    
    callback->dumper.steps_data.n_max_steps = 100;
    callback->dumper.steps_data.n_steps = 0;
    calblack->dumper.steps_data.data = 
        memory_malloc(sizeof(DumpNetworkData) * callback->dumper.steps_data.n_max_steps,
                      "callback_network_dumper_create data");
    check_memory(callback->dumper.steps.data);
    
    return callback;
    
    error:
    // TODO:
    return NULL;
}


internal void
callback_network_dumper_drestroy(Callback* callback) {
}