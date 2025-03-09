import matplotlib.pyplot as plt

from callbacks.callback import Callback

class WeightHistCallback(Callback):

    def __init__(self):
        self.weights = []
        self.max_time = 0

    def before_example(self, net):
        self.weights = []
        self.max_time = 0

    def update(self, net, time):
        pass

    def after_example(self, net, winners, example_i):
        if example_i % 10 != 0:
            return

        for layer in net.layers:
            for neuron in layer.neurons:
                for synapse in neuron.in_synapses:
                    self.weights.append(synapse.w)

        plt.hist(self.weights)
        plt.title(f'{example_i}')
        plt.show()

        plt.cla()
        plt.clf()
        plt.close()