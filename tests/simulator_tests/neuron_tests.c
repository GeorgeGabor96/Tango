#include "common.h"
#include "tests.h"
#include "simulator/neuron.h"
#include "simulator/neuron.c"


internal TestStatus
neuron_create_destroy_test() {
    TestStatus status = TEST_FAILED;
    
    NeuronCls* cls = neuron_cls_create_lif_refract(1);
    assert(cls != NULL, "cls is NULL");
    assert(cls->type == NEURON_LIF_REFRACT, "cls->type is %s, not NEURON_LIF_REFRACT",
           neuron_type_get_c_str(cls->type));
    assert(cls->lif_refract_cls.refract_time == 1,
           "cls->lif_refract_cls.refract_time should be 1 not %u",
           cls->lif_refract_cls.refract_time);
    
    Array* in_synapses_ref = array_create(2, sizeof(Synapse*));
    Array* out_synapses_ref = array_create(2, sizeof(Synapse*));
    Neuron* neuron = neuron_create(cls, in_synapses_ref, out_synapses_ref);
    assert(neuron != NULL, "neuron is null");
    assert(neuron->cls == cls, "neuron->cls is %p not %p", neuron->cls, cls);
    assert(neuron->voltage == NEURON_LIF_VOLTAGE_REST, "neuron->voltage is %f not %f",
           neuron->voltage, NEURON_LIF_VOLTAGE_REST);
    assert(float_equal(neuron->epsc, 0.0f) == TRUE, "neuron->epsc is %f not 0.0",
           neuron->epsc);
    assert(float_equal(neuron->ipsc, 0.0f) == TRUE, "neuron->ipsc is %f not 0.0",
           neuron->ipsc);
    assert(neuron->in_synapses_ref == in_synapses_ref,
           "neuron->in_synapses_ref is %p not %p",
           neuron->in_synapses_ref, in_synapses_ref);
    assert(neuron->out_synapses_ref == out_synapses_ref,
           "neuron->out_synapses_ref is %p not %p",
           neuron->out_synapses_ref, out_synapses_ref);
    assert(neuron->spike == FALSE, "neuron->spike should be FALSE");
    assert(neuron->lif_refract.last_spike_time == 0,
           "neuron->lif_refract.last_spike_time is %u not 0",
           neuron->lif_refract.last_spike_time);
    
    neuron_cls_destroy(cls);
    neuron_destroy(neuron);
    
    // EDGE cases
    neuron = neuron_create(NULL, in_synapses_ref, out_synapses_ref);
    assert(neuron == NULL, "neuron should be NULL for NULL cls");
    neuron = neuron_create(cls, NULL, out_synapses_ref);
    assert(neuron == NULL, "neuron should be NULL for NULL in_synapses_ref");
    neuron = neuron_create(cls, in_synapses_ref, NULL);
    assert(neuron == NULL, "neuron should be NULL for NULL out_synapses_ref");
    neuron_cls_destroy(NULL);
    neuron_destroy(NULL);
    
    array_destroy(in_synapses_ref, NULL);
    array_destroy(out_synapses_ref, NULL);
    
    assert(memory_leak() == FALSE, "Memory Leak");
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
neuron_step_test() {
    TestStatus status = TEST_FAILED;
    
    SynapseCls* synapse_cls = synapse_cls_create(SYNAPSE_VOLTAGE, 0.0f, 1.0f, 20, 2);
    Synapse* in_synapse_1 = synapse_create(synapse_cls, 1.0f);
    Synapse* in_synapse_2 = synapse_create(synapse_cls, 1.0f);
    Synapse* out_synapse_1 = synapse_create(synapse_cls, 1.0f);
    Synapse* out_synapse_2 = synapse_create(synapse_cls, 1.0f);
    Array* in_synapses_ref = array_create(2, sizeof(Synapse*));
    Array* out_synapses_ref = array_create(2, sizeof(Synapse*));
    array_set(in_synapses_ref, &in_synapse_1, 0);
    array_set(in_synapses_ref, &in_synapse_2, 1);
    array_set(out_synapses_ref, &out_synapse_1, 0);
    array_set(out_synapses_ref, &out_synapse_2, 1);
    
    NeuronCls* neuron_cls = neuron_cls_create_lif_refract(1);
    Neuron* neuron = neuron_create(neuron_cls, in_synapses_ref, out_synapses_ref);
    
    /*****************
* TEST neuron_step
*****************/
    // NOTE: if the input synapses don't produce current the neuron should not spike
    neuron_step(neuron, 100);
    assert(neuron->spike == FALSE, "neuron should not produce a spike");
    assert(queue_is_empty(out_synapse_1->spike_times) == TRUE,
           "out synapse 1 should not have spikes in it");
    assert(queue_is_empty(out_synapse_2->spike_times) == TRUE,
           "out synapse 2 should not ahve spikes in it");
    
    // NOTE: if both the synapse have current because they are excitatory they should make
    // NOTE: the neuron spike
    synapse_add_spike_time(in_synapse_1, 99);
    synapse_add_spike_time(in_synapse_2, 99);
    neuron_step(neuron, 101);
    assert(queue_is_empty(in_synapse_1->spike_times) == TRUE,
           "in synapse 1 should not have any more spikes");
    assert(queue_is_empty(in_synapse_2->spike_times) == TRUE,
           "in synapse 2 should not have any more spikes");
    
    assert(neuron->spike == TRUE, "neuron should produce a spike");
    
    assert(queue_is_empty(out_synapse_1->spike_times) == FALSE,
           "out synapse 1 should have spikes");
    u32 spike_time = *((u32*)queue_head(out_synapse_1->spike_times)); 
    assert(spike_time == 103, "out synapse 1 should have 103 as the spike time");
    
    assert(queue_is_empty(out_synapse_2->spike_times) == FALSE,
           "out synapse 2 should have spikes");
    spike_time = *((u32*)queue_head(out_synapse_2->spike_times)); 
    assert(spike_time == 103, "out synapse 2 should have 103 as the spike time");
    
    // NOTE: clear the output synapses
    synapse_step(out_synapse_1, 103);
    synapse_step(out_synapse_2, 103);
    assert(queue_is_empty(out_synapse_1->spike_times) == TRUE,
           "out synapse 1 should not have any more spikes");
    assert(queue_is_empty(out_synapse_2->spike_times) == TRUE,
           "out synapse 2 should not have any more spikes");
    
    
    /*****************************
* TEST neuron_step_force_spike
*****************************/
    // NOTE: force spike
    neuron_step_force_spike(neuron, 200);
    assert(neuron->spike == TRUE, "neuron should produce a spike");
    
    assert(queue_is_empty(out_synapse_1->spike_times) == FALSE,
           "out synapse 1 should have spikes");
    spike_time = *((u32*)queue_head(out_synapse_1->spike_times)); 
    assert(spike_time == 202, "out synapse 1 should have 202 as the spike time");
    
    assert(queue_is_empty(out_synapse_2->spike_times) == FALSE,
           "out synapse 2 should have spikes");
    spike_time = *((u32*)queue_head(out_synapse_2->spike_times)); 
    assert(spike_time == 202, "out synapse 2 should have 202 as the spike time");
    
    // NOTE: clear the output synapses
    synapse_step(out_synapse_1, 202);
    synapse_step(out_synapse_2, 202);
    assert(queue_is_empty(out_synapse_1->spike_times) == TRUE,
           "out synapse 1 should not have any more spikes");
    assert(queue_is_empty(out_synapse_2->spike_times) == TRUE,
           "out synapse 2 should not have any more spikes");
    
    /********************************
    * TEST neuron_step_inject_current
    ********************************/
    neuron_step_inject_current(neuron, 100.0f, 300);
    assert(neuron->spike == TRUE, "neuron should produce a spike");
    
    assert(queue_is_empty(out_synapse_1->spike_times) == FALSE,
           "out synapse 1 should have spikes");
    spike_time = *((u32*)queue_head(out_synapse_1->spike_times)); 
    assert(spike_time == 302, "out synapse 1 should have 302 as the spike time");
    
    assert(queue_is_empty(out_synapse_2->spike_times) == FALSE,
           "out synapse 2 should have spikes");
    spike_time = *((u32*)queue_head(out_synapse_2->spike_times)); 
    assert(spike_time == 302, "out synapse 2 should have 302 as the spike time");
    
    // NOTE: clear the output synapses
    synapse_step(out_synapse_1, 302);
    synapse_step(out_synapse_2, 302);
    assert(queue_is_empty(out_synapse_1->spike_times) == TRUE,
           "out synapse 1 should not have any more spikes");
    assert(queue_is_empty(out_synapse_2->spike_times) == TRUE,
           "out synapse 2 should not have any more spikes");
    
    neuron_destroy(neuron);
    neuron_cls_destroy(neuron_cls);
    synapse_destroy(in_synapse_1);
    synapse_destroy(in_synapse_2);
    synapse_destroy(out_synapse_1);
    synapse_destroy(out_synapse_2);
    array_destroy(in_synapses_ref, NULL);
    array_destroy(out_synapses_ref, NULL);
    synapse_cls_destroy(synapse_cls);
    
    // EDGE CASES
    neuron_step(NULL, 100);
    neuron_step_force_spike(NULL, 100);
    neuron_step_inject_current(NULL, 100.0f, 100);
    
    assert(memory_leak() == FALSE, "Memory leak");
    status = TEST_SUCCESS;
    error:
    return status;
}