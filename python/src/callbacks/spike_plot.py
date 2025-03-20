import matplotlib.pyplot as plt

from callbacks.callback import Callback
from utils import get_spikes_for_times

class SpikePlotCallback(Callback):

    def __init__(self):
        self.neurons_spike_times = []
        self.n_neurons = 0
        self.max_time = 0

    def before_example(self, net):
        self.neurons_spike_times = []
        self.max_time = 0
        self.n_neurons = net.get_n_neurons()

        for i in range(self.n_neurons):
            self.neurons_spike_times.append([])

    def update(self, net, time):
        neuron_i = 0
        for layer in net.layers:
            for neuron in layer.neurons:
                if neuron.spike:
                    self.neurons_spike_times[neuron_i].append(time)
                neuron_i += 1
        self.max_time = time

    def after_example(self, net, winners, example_i):
        if example_i % 10 != 0:
            return

        _, ax = plt.subplots(self.n_neurons, sharex='all')
        plt.xlim(0, self.max_time)

        for i, neuron_spike_times in enumerate(self.neurons_spike_times):
            ax[i].scatter(neuron_spike_times, get_spikes_for_times(neuron_spike_times), marker='|')

        plt.show(block=False)
        plt.pause(1)
        plt.close()

        plt.cla()
        plt.clf()