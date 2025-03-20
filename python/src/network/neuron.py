
from numba import jit

class Neuron:
    def __init__(self, tag = ''):
        self.tag = tag
        self.th = 1.0
        self.rest = 0.0
        self.spike = False
        self.spike_times = []

        self.in_synapses = []
        self.out_synapses = []
        self.reset()

    def reset(self):
        self.injected_current = 0.0
        self.v = 0.0
        self.last_spike_time = -1
        self.spike = False
        self.spike_times = []

        for synapse in self.in_synapses:
            synapse.reset()

    def add_in_synapse(self, synapse):
        self.in_synapses.append(synapse)

    def add_out_synapse(self, synapse):
        self.out_synapses.append(synapse)

    def inject_current(self, current):
        self.injected_current = current

    def update(self, time):
        current = self.injected_current
        current_eq = ''
        for synapse in self.in_synapses:
            current += synapse.get_current()
            current_eq += str(synapse.get_current()) + '+'

        if self.tag == 'DEBUG':
            print(current_eq, '=', current)

        self.v = self.v * 0.5 + current

        if self.v >= self.th and time - self.last_spike_time >= 3:
            self.v = self.rest
            self.last_spike_time = time
            self.spike = True
            self.spike_times.append(time)
        else:
            self.spike = False

        for synapse in self.out_synapses:
            synapse.update(self.spike)

        return