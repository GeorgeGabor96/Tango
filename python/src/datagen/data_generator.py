from abc import ABC, abstractmethod

class DataGenerator(ABC):

    @abstractmethod
    def get_currents(self, net, time):
        pass