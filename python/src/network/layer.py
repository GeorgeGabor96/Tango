import random

from network.neuron import Neuron
from network.synapse import Synapse

class Layer:
    def __init__(self, n_neurons):
        self.neurons = [Neuron() for _ in range(n_neurons)]

    def inject_currents(self, currents):
        assert len(currents) == len(self.neurons)
        for i in range(len(currents)):
            self.neurons[i].inject_current(currents[i])

    def update(self, time):
        for neuron in self.neurons:
            neuron.update(time)


def link_layers(layer1, layer2):
    n_synapses_created = 0
    for neuron1 in layer1.neurons:
        for neuron2 in layer2.neurons:
            synapse = Synapse(w = random.uniform(0, 1))
            neuron1.add_out_synapse(synapse)
            neuron2.add_in_synapse(synapse)
            n_synapses_created += 1
    return n_synapses_created
