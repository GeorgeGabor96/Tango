#include "simulator/data_inputs.h"


/***********************
* DATA INPUTS
***********************/
internal DataInputs*
data_gen_inputs_create(DataSample* sample, Network* network, u32 time) {
    check(sample != NULL, "sample is NULL");
    check(network != NULL, "network is NULL");
    
    u32 i = 0;
    DataInput* input = NULL;
    DataInputs* inputs = (DataInputs*) memory_malloc(sizeof(*inputs));
    check_memory(inputs);
    inputs->n_inputs = network->n_in_layers;
    inputs->inputs = (DataInput*) memory_malloc(sizeof(DataInput) * inputs->n_inputs);
    check_memory(inputs->inputs);
    
    if (sample->type == DATA_SAMPLE_RANDOM_SPIKES) {
        for (i = 0; i < inputs->n_inputs; ++i) {
            input = inputs->inputs + i;
            input->type = DATA_INPUT_SPIKES;
            input->data = (bool*) memory_calloc(sizeof(bool) * sample->duration);
            check_memory(input->data);
            // TODO: need a random number generator, don't use the one from C std
            input->n_values = sample->duration; 
        }
    } else if (sample->type == DATA_SAMPLE_CONSTANT_CURRENT) {
        f32* currents = NULL;
        for (i = 0; i < inputs->n_inputs; ++i) {
            input = inputs->inputs + i;
            
            currents = (f32*)memory_malloc(sizeof(f32) * sample->duration);
            check_memory(input->data);
            for (j = 0; j < input->n_values; ++j)
                currents[j] = sample->sample_const_current.value;
            
            input->type = DATA_INPUT_CURRENT;
            input->data = currents;
            input->n_value = sample->duration;
        }
    } else {
        log_error("Unknown data sample type %u", sample->type);
    }
    
    return inputs;
    
    error:
    // NOTE: Don't free anything yet, when its stable we will free all the things
    return NULL;
}


internal void
data_gen_inputs_destroy(DataInputs* inputs) {
    check(inputs != NULL, "inputs is NULL");
    DataInput* input = NULL;
    u32 i = 0;
    
    for (i = 0; i < inputs->n_inputs; ++i) {
        input = inputs->inputs + i;
        memory_free(input->data);
        memset(input, 0, sizeof(*input));
    }
    memset(inputs, 0, sizeof(*inputs));
    memory_free(inputs);
    
    error;
    return;
}
