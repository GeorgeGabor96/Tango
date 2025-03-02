from datagen.data_generator import DataGenerator, DataExample
import random


class XorDataExample(DataExample):
    def get_currents(self, net, time):
        n_currents = len(net.get_in_layer().neurons)

        first_group = n_currents // 2
        second_group = n_currents - first_group

        # Neurons in first group should spike
        spike_change = 0.5
        currents = []
        for _ in range(first_group):
            if random.uniform(0, 1) < spike_change:
                currents.append(1)
            else:
                currents.append(0)

        # Neurons in second group should not spike
        for _ in range(second_group):
            currents.append(0)

        return currents


class XorDataGenerator(DataGenerator):
    def __init__(self):
        pass

    def get_example(self, example_i):
        example = XorDataExample()
        return example