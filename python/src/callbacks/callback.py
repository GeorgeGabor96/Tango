from abc import ABC, abstractmethod

class Callback(ABC):

    @abstractmethod
    def before_run(self, net):
        pass

    @abstractmethod
    def update(self, net, time):
        pass

    @abstractmethod
    def after_run(self, net):
        pass