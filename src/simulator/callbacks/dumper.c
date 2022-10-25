typedef struct DumpStepsData {
    u32 n_max_steps;
    u32 n_steps;
    DumpNetworkData* net_steps_data; // NOTE: needs to be resized
} DumpStepsData;


void dumper_update_steps(DumpStepsData* steps, Network* network);


typedef struct DumpNetworkData {
    u32 n_layers;
    DumpLayerData* layers_data; // NOTE: Should know the number of layers, no resize
} DumpNetworkData;

void dumper_update_network_data(DumpNetworkData* network_data, Network* network);


typedef struct DumpLayerData {
    u32 n_neurons;
    u32 n_max_neurons;
    DumpNeuronData* neurons_data;
} DumpLayerData;

void dumper_update_layer_data(DumpLayerData* layer_data, Layer* layer);


typedef struct DumpNeuronData {
    u32 n_steps;
    u32 n_max_steps;
    f32* voltage;
    bool* spike;
    f32* psc;
    f32* epsc;
    f32* ipsc;
} DumpNeuronData;

// TODO/NOTE: TO create one binary file fast, we should have the data (the
// main part) in the format of the binary file, meaning that for one neuron
// we should directly have everything.


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
callback_update_dumper(Callback* dumper, Network* network) {
    check(dumper != NULL, "dumper is NULL");
    check(callback->type == CALLBACK_NETWORK_DUMPER,
          "invalid callback type");
    check(network != NULL, "network is NULL");
    
    DumpStepsData* steps = &(callback->dumper.steps);
    if (steps->n_steps == steps->n_max_steps) {
        // TODO: resize array
    }
    
    DumpNetworkData* network_data = steps->data[steps->n_steps];
    ++(steps->n_steps);
    check(network_data->n_layers == network->n_layers, "INVALID NUMBER OF LAYRES");
    
    for (u32 i = 0; i < network->n_layers; ++i) {
        Layer* layer = network->layers[i];
        DumpLayerData* layer_data = &(network_data->data[i]);
        
        
        
    }
    
    
    error:
    return;
}