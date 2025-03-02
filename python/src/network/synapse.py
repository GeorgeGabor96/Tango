class Synapse:
    def __init__(self, w):
        self.conductance = 0.0
        self.w = w

        self.in_neuron = None
        self.out_neuron = None

    def set_in_neuron(self, neuron):
        self.in_neuron = neuron

    def set_out_neuron(self, neuron):
        self.out_neuron = neuron

    def update(self, spike):
        if spike == True:
            self.conductance += 1.0
        else:
            if self.conductance < 0.01:
                self.conductance == 0
            else:
                self.conductance *= 0.5

    def get_current(self):
        c = self.w * self.conductance
        return c