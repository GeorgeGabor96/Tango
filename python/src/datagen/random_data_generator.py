import random

from datagen.data_generator import DataGenerator, DataExample


class RandomDataExample(DataExample):
    def __init__(self):
        pass

    def get_currents(self, net, time):
        n_currents = len(net.get_in_layer().neurons)
        currents = [random.uniform(0, 1) for _ in range(n_currents)]
        return currents

class RandomDataGenerator(DataGenerator):
    def __init__(self):
        pass

    def get_example(self, example_i):
        example = RandomDataExample()
        return example