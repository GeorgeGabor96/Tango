#include "common.h"
#include "tests.h"
#include "simulator/synapse.h"
#include "simulator/synapse.c"


internal TestStatus
synapse_create_destroy_test() {
    TestStatus status = TEST_FAILED;
    
    SynapseCls* cls = synapse_cls_create(SYNAPSE_VOLTAGE, -90.0f, 1.0f, 20.0f, 3);
    assert(cls != NULL, "cls is NULL");
    assert(cls->type == SYNAPSE_VOLTAGE, "cls->type should be %s not %s",
           synapse_type_get_c_str(SYNAPSE_VOLTAGE),
           synapse_type_get_c_str(cls->type));
    assert(float_equal(cls->rev_potential, -90.0f),
           "cls->rev_potential is %f not -90.0", cls->rev_potential);
    assert(float_equal(cls->amp, 1.0f), "cls->amp is %f not 1.0", cls->amp);
    f32 tau_exp_true = (f32)math_op_exp((f64)-1 / 20.0f);
    assert(float_equal(cls->tau_exp, tau_exp_true), "cls->tau_exp is %f not %f",
           cls->tau_exp, tau_exp_true);
    assert(cls->delay == 3, "cls->delay is %u not 3", cls->delay);
    
    Synapse* synapse = synapse_create(cls, 1.0f);
    assert(synapse != NULL, "synapse is NULL");
    assert(synapse->cls == cls, "synapse->cls is %p not %p", synapse->cls, cls);
    assert(float_equal(synapse->weight, 1.0f), "synapse->weight is %f not 1.0", synapse->weight);
    assert(float_equal(synapse->conductance, 0.0f), "synapse->conductance is %f not 0.0f",
           synapse->conductance);
    assert(synapse->spike_times != NULL, "synapse->spike_times is NULL");
    
    synapse_cls_destroy(cls);
    synapse_destroy(synapse);
    
    assert(memory_leak() == FALSE, "Memory Leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}


internal TestStatus
synapse_behaviour_test() {
    TestStatus status = TEST_FAILED;
    
    SynapseCls* cls = synapse_cls_create(SYNAPSE_VOLTAGE, -90.0f, 1.0f, 20.0f, 3);
    Synapse* synapse = synapse_create(cls, 1.0f);
    
    // NOTE: a synapse with no activity should produce 0 psc
    f32 current = synapse_compute_psc(synapse, -50.f);
    assert(float_equal(current, 0.0f), "synapse should produce 0 current not %f", current);
    
    // NOTE: test that the synapse can hold 3 spikes
    // NOTE: the synapse should only modify its conductance for times 103, 104, 105
    f32 conductance = synapse->conductance;
    assert(float_equal(conductance, 0.0f), "conductance should be 0 not %f", conductance);
    
    synapse_add_spike_time(synapse, 100);
    synapse_step(synapse, 100);
    conductance = synapse->conductance;
    assert(float_equal(conductance, 0.0f), "for time 100 conductance should be 0 not %f",
           conductance);
    
    synapse_add_spike_time(synapse, 101);
    synapse_step(synapse, 101);
    conductance = synapse->conductance;
    assert(float_equal(conductance, 0.0f), "for time 101 conductance should be 0 not %f",
           conductance);
    
    synapse_add_spike_time(synapse, 102);
    synapse_step(synapse, 102);
    conductance = synapse->conductance;
    assert(float_equal(conductance, 0.0f), "for time 102 conductance should be 0 not %f",
           conductance);
    
    // NOTE: from the next 3 steps the conductance needs to increase by 1 every time
    synapse_add_spike_time(synapse, 103);
    synapse_step(synapse, 103);
    conductance = synapse->conductance;
    assert(float_equal(conductance, 1.0f), "for time 103 conductance should be 1 not %f",
           conductance);
    
    synapse_step(synapse, 104);
    conductance = synapse->conductance;
    assert(float_equal(conductance, 2.0f), "for time 104 conductance should be 2 not %f",
           conductance);
    
    synapse_step(synapse, 105);
    conductance = synapse->conductance;
    assert(float_equal(conductance, 3.0f), "for time 105 conductance should be 3 not %f",
           conductance);
    
    synapse_step(synapse, 106);
    conductance = synapse->conductance;
    assert(float_equal(conductance, 4.0f), "for time 106 conductance should be 4 not %f",
           conductance);
    
    // NOTE: from this point the conductance should decrease after every step
    f32 old_conductance = synapse->conductance;
    synapse_step(synapse, 107);
    conductance = synapse->conductance;
    assert(old_conductance > conductance,
           "for time 107 conductance should be smaller than %f, not %f",
           old_conductance, conductance);
    
    old_conductance = synapse->conductance;
    synapse_step(synapse, 108);
    conductance = synapse->conductance;
    assert(old_conductance > conductance,
           "for time 108 conductance should be smaller than %f, not %f",
           old_conductance, conductance);
    
    old_conductance = synapse->conductance;
    synapse_step(synapse, 109);
    conductance = synapse->conductance;
    assert(old_conductance > conductance,
           "for time 109 conductance should be smaller than %f, not %f",
           old_conductance, conductance);
    
    synapse_destroy(synapse);
    synapse_cls_destroy(cls);
    
    assert(memory_leak() == FALSE, "Memory Leak");
    
    status = TEST_SUCCESS;
    error:
    return status;
}