from abc import ABC, abstractmethod
import random

class SynapseWeightInitializer(ABC):

    @abstractmethod
    def get_weight(self):
        pass

class UniformWeightInitializer(SynapseWeightInitializer):
    def get_weight(self):
        result = random.uniform(0, 1)
        return result

class ZeroWeightInitializer(SynapseWeightInitializer):
    def get_weight(self):
        return 0