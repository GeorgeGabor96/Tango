from datagen.data_generator import DataGenerator, DataExample
import random

def get_spike():
    spike_change = 0.5
    if random.uniform(0, 1) < spike_change:
        return 1
    return 0

# Note: use 4 input neurons
# First - normal first input to xor
# Second - normal first input to xor
# Third - spikes when First doesn't
# Forth - spikes when Second doesn't

class FirstOffSecondOffDataExample(DataExample):
    def get_currents(self, net, time):
        n_currents = len(net.get_in_layer().neurons)
        assert n_currents == 4

        currents = [
            0,
            0,
            get_spike(),
            get_spike()
        ]

        return currents

    def get_winners(self, net, time):
        n_out_neurons = len(net.get_out_layer().neurons)
        assert n_out_neurons == 2

        winners = [i for i in range(n_out_neurons // 2)]
        return [1]

class FirstOnSecondOffDataExample(DataExample):
    def get_currents(self, net, time):
        n_currents = len(net.get_in_layer().neurons)
        assert n_currents == 4

        currents = [
            get_spike(),
            0,
            0,
            get_spike()
        ]

        return currents

    def get_winners(self, net, time):
        n_out_neurons = len(net.get_out_layer().neurons)
        assert n_out_neurons == 2

        winners = [i for i in range(n_out_neurons // 2)]
        return [0]

class FirstOffSecondOnDataExample(DataExample):
    def get_currents(self, net, time):
        n_currents = len(net.get_in_layer().neurons)
        assert n_currents == 4

        currents = [
            0,
            get_spike(),
            get_spike(),
            0,
        ]

        return currents

    def get_winners(self, net, time):
        n_out_neurons = len(net.get_out_layer().neurons)
        assert n_out_neurons == 2

        winners = [i for i in range(n_out_neurons // 2)]
        return [0]

class FirstOnSecondOnDataExample(DataExample):
    def get_currents(self, net, time):
        n_currents = len(net.get_in_layer().neurons)
        assert n_currents == 4

        currents = [
            get_spike(),
            get_spike(),
            0,
            0,
        ]

        return currents

    def get_winners(self, net, time):
        n_out_neurons = len(net.get_out_layer().neurons)
        assert n_out_neurons == 2

        winners = [i for i in range(n_out_neurons // 2)]
        return [1]


class XorDataGenerator(DataGenerator):
    def __init__(self):
        pass

    def get_example(self, example_i):

        if random.uniform(0, 1) > 0.5:
            example = FirstOffSecondOffDataExample()
        else:
            example = FirstOnSecondOffDataExample()

        return example