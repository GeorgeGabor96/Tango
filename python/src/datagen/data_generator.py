from abc import ABC, abstractmethod


class DataExample(ABC):
    @abstractmethod
    def get_currents(self, net, time):
        pass

class DataGenerator(ABC):
    @abstractmethod
    def get_example(self, example_i) -> DataExample:
        pass