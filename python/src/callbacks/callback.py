from abc import ABC, abstractmethod

class Callback(ABC):

    @abstractmethod
    def before_example(self, net):
        pass

    @abstractmethod
    def update(self, net, time):
        pass

    @abstractmethod
    def after_example(self, net, winners, example_i):
        pass