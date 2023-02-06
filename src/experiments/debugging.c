#include "tango.h"


void build_network(void* exp) {
    f32 connect_chance = 0.25;

    tango_create_neuron_cls_lif_refract(exp, "LIF refract cls", 5);
    tango_create_synapse_cls(exp, "AMPA", SYNAPSE_VOLTAGE, 0.0f, 0.1f, 1, 10);
    tango_create_synapse_cls(exp, "GABA_A", SYNAPSE_VOLTAGE, -90.0F, 0.1F, 6, 10);

    tango_create_layer(exp, "input_exci", LAYER_DENSE, 90, "LIF refract cls", TRUE, FALSE);
    tango_create_layer(exp, "input_inhi", LAYER_DENSE, 10, "LIF refract cls", TRUE, FALSE);

    char last_exci_name[100] = { 0 };
    sprintf(last_exci_name, "input_exci");
    char last_inhi_name[100] = { 0 };
    sprintf(last_inhi_name, "input_inhi");

    for (u32 i = 1; i < 10; ++i) {
        char exci_name[100] = { 0 };
        sprintf(exci_name, "layer_%d_exci", i);
        tango_create_layer(exp, exci_name, LAYER_DENSE, 90, "LIF refract cls", FALSE, FALSE);

        char inhi_name[100] = { 0 };
        sprintf(inhi_name, "layer_%d_inhi", i);
        tango_create_layer(exp, inhi_name, LAYER_DENSE, 10, "LIF refract cls", FALSE, FALSE);

        tango_link_layers(exp, exci_name, last_exci_name, "AMPA", 1, connect_chance);
        tango_link_layers(exp, exci_name, last_inhi_name, "GABA_A", 1, connect_chance);
        tango_link_layers(exp, inhi_name, last_exci_name, "AMPA", 1, connect_chance);
        tango_link_layers(exp, inhi_name, last_inhi_name, "GABA_A", 1, connect_chance);

        sprintf(last_exci_name, "layer_%d_exci", i);
        sprintf(last_inhi_name, "layer_%d_inhi", i);
    }

    tango_create_layer(exp, "output_exci", LAYER_DENSE, 100, "LIF refract cls", FALSE, TRUE);
    tango_link_layers(exp, "output_exci", last_exci_name, "AMPA", 1, connect_chance);
    tango_link_layers(exp, "output_exci", last_inhi_name, "GABA_A", 1, connect_chance);

    tango_build_network(exp);
    tango_show_network(exp);
}



int main() {
    void* exp = tango_create(4, 1234, "D:\\repos\\Tango_outputs\\synfire_chain");
    build_network(exp);

    tango_create_data_spike_pulses(exp, 2, 1000, 100, 20, 50, 0.1f, 0.01f);
    tango_create_callback_dumper(exp);

    tango_learn(exp);

    tango_destroy(exp);

    return 0;
}
