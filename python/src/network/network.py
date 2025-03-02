from network.layer import Layer, link_layers
from network.weight_init import SynapseWeightInitializer

class Network:
    def __init__(self):
        self.layers = []
        self.n_neurons = 0
        self.is_built = False

    def get_in_layer(self):
        result = None
        if len(self.layers) > 0:
            result = self.layers[0]
        return result

    def get_out_layer(self):
        result = None
        if len(self.layers) > 0:
            result = self.layers[-1]
        return result

    def get_n_neurons(self):
        assert self.is_built is True

        return self.n_neurons

    def add_layer(self, layer):
        self.layers.append(layer)

    def build(self, weight_initializer: SynapseWeightInitializer):
        assert len(self.layers) > 0

        # assume sequential model for now
        for i in range(len(self.layers) - 1):
            layer_in = self.layers[i]
            layer_out = self.layers[i + 1]
            link_layers(layer_in, layer_out, weight_initializer)

        for layer in self.layers:
            self.n_neurons += len(layer.neurons)

        self.is_built = True

    def update(self, currents, time):
        in_layer = self.get_in_layer()
        in_layer.inject_currents(currents)

        for layer in self.layers:
            layer.update(time)

    def reset(self):
        for layer in self.layers:
            layer.reset()