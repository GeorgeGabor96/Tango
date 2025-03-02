from network.neuron import Neuron
from network.synapse import Synapse
from network.weight_init import SynapseWeightInitializer

class Layer:
    def __init__(self, n_neurons, name):
        self.neurons = [Neuron() for _ in range(n_neurons)]
        self.name = name

    def inject_currents(self, currents):
        assert len(currents) == len(self.neurons)
        for i in range(len(currents)):
            self.neurons[i].inject_current(currents[i])

    def update(self, time):
        for neuron in self.neurons:
            neuron.update(time)

    def reset(self):
        for neuron in self.neurons:
            neuron.reset()

def link_layers(layer1, layer2, weight_initializer: SynapseWeightInitializer):
    n_synapses_created = 0
    for neuron1 in layer1.neurons:
        for neuron2 in layer2.neurons:
            synapse = Synapse(w = weight_initializer.get_weight())
            neuron1.add_out_synapse(synapse)
            neuron2.add_in_synapse(synapse)
            synapse.set_in_neuron(neuron1)
            synapse.set_out_neuron(neuron2)
            n_synapses_created += 1
    return n_synapses_created
