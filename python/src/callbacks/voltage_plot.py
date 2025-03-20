import matplotlib.pyplot as plt

from callbacks.callback import Callback

def get_times(max_time):
    result = [i for i in range(max_time)]
    return result

class VoltagePlotCallback(Callback):

    def __init__(self):
        self.neurons_voltages = []
        self.n_neurons = 0
        self.max_time = 0

    def before_example(self, net):
        self.neurons_voltages = []
        self.max_time = 0
        self.n_neurons = net.get_n_neurons()

        for i in range(self.n_neurons):
            self.neurons_voltages.append([])

    def update(self, net, time):
        neuron_i = 0
        for layer in net.layers:
            for neuron in layer.neurons:
                self.neurons_voltages[neuron_i].append(neuron.v)
                neuron_i += 1
        self.max_time = time

    def after_example(self, net, winners, example_i):
        _, ax = plt.subplots(self.n_neurons, sharex='all')
        plt.xlim(0, self.max_time)

        for i, neuron_voltages in enumerate(self.neurons_voltages):
            ax[i].plot(get_times(self.max_time + 1), neuron_voltages)

        plt.show(block=False)
        plt.pause(1)
        plt.close()

        plt.cla()
        plt.clf()