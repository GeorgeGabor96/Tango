import matplotlib.pyplot as plt

def get_spikes_for_times(times):
    return [1 for _ in times]

class Neuron:
    def __init__(self, tag = ''):
        self.tag = tag
        self.th = 1.0
        self.rest = 0.0

        self.in_synapses = []
        self.out_synapses = []

        self.injected_current = 0.0
        self.v = 0.0
        self.last_spike_time = -1
        self.spike = False
        self.spike_times = []

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

def test_behaviour():
    syn1 = Synapse(0.2)
    syn2 = Synapse(0.5)
    out = Neuron()


    in1_spikes = []
    syn1_conductance = []
    in2_spikes = []
    syn2_conductance = []
    currents = []
    out_spikes = []
    out_voltages = []
    for i in range(100):
        if i < 25:
            in1_spikes.append(0)
            in2_spikes.append(0)
            pass

        elif i < 50:
            if i % 3 == 0:
                in1_spikes.append(1)
            else:
                in1_spikes.append(0)
            in2_spikes.append(0)
            pass

        elif i < 75:
            in1_spikes.append(0)
            if i % 3 == 0:
                in2_spikes.append(1)
            else:
                in2_spikes.append(0)
            pass

        else:
            if i % 3 == 0:
                in1_spikes.append(1)
                in2_spikes.append(1)
            else:
                in1_spikes.append(0)
                in2_spikes.append(0)

        syn1.update(in1_spikes[i])
        syn1_conductance.append(syn1.conductance)

        syn2.update(in2_spikes[i])
        syn2_conductance.append(syn2.conductance)

        current = syn1.get_current() + syn2.get_current()
        currents.append(current)

        out.update(current)
        out_spikes.append(out.spike)
        out_voltages.append(out.v)

    times = [i for i in range(100)]

    fig, ax = plt.subplots(7, sharex='all')
    ax[0].scatter(times, in1_spikes)
    ax[1].scatter(times, in2_spikes)
    ax[2].plot(times, syn1_conductance)
    ax[3].plot(times, syn2_conductance)
    ax[4].plot(times, currents)
    ax[5].plot(times, out_voltages)
    ax[6].scatter(times, out_spikes)
    plt.show()

def test_xor():
    # input poz
    neuron_in_1 = Neuron()
    neuron_in_2 = Neuron()

    # input neg
    neuron_in_3 = Neuron()
    neuron_in_4 = Neuron()

    # Hidden layer poz
    synapse_in_1_hidden_1 = Synapse(w = 1)
    synapse_in_2_hidden_1 = Synapse(w = -1)

    synapse_in_1_hidden_2 = Synapse(w = -1)
    synapse_in_2_hidden_2 = Synapse(w = 1)

    neuron_hidden_1 = Neuron()
    neuron_hidden_2 = Neuron()

    neuron_in_1.add_out_synapse(synapse_in_1_hidden_1)
    neuron_in_1.add_out_synapse(synapse_in_1_hidden_2)

    neuron_in_2.add_out_synapse(synapse_in_2_hidden_1)
    neuron_in_2.add_out_synapse(synapse_in_2_hidden_2)

    neuron_hidden_1.add_in_synapse(synapse_in_1_hidden_1)
    neuron_hidden_1.add_in_synapse(synapse_in_2_hidden_1)

    neuron_hidden_2.add_in_synapse(synapse_in_1_hidden_2)
    neuron_hidden_2.add_in_synapse(synapse_in_2_hidden_2)

    # hidden layer neg
    neuron_hidden_3 = Neuron()
    synapse_in_3_hidden_3 = Synapse(0)
    synapse_in_4_hidden_3 = Synapse(0)

    # Output Layer
    synapse_hidden_1_output_xor = Synapse(w = 1)
    synapse_hidden_2_output_xor = Synapse(w = 1)
    neuron_output_xor = Neuron()

    neuron_hidden_1.add_out_synapse(synapse_hidden_1_output_xor)
    neuron_hidden_2.add_out_synapse(synapse_hidden_2_output_xor)

    neuron_output_xor.add_in_synapse(synapse_hidden_1_output_xor)
    neuron_output_xor.add_in_synapse(synapse_hidden_2_output_xor)

    for time in range(100):
        neuron_in_1_inject_current = 1 if (25 <= time < 50 or 75 <= time < 100) and time % 3 == 0 else 0
        neuron_in_1.inject_current(neuron_in_1_inject_current)
        neuron_in_1.update(time)

        neuron_in_2_inject_current = 1 if (50 <= time < 100) and time % 3 == 0 else 0
        neuron_in_2.inject_current(neuron_in_2_inject_current)
        neuron_in_2.update(time)

        neuron_hidden_1.update(time)
        neuron_hidden_2.update(time)

        neuron_output_xor.update(time)

    _, ax = plt.subplots(5, sharex='all')
    plt.xlim(0, 100)
    ax[0].scatter(neuron_in_1.spike_times, get_spikes_for_times(neuron_in_1.spike_times), marker='|')
    ax[1].scatter(neuron_in_2.spike_times, get_spikes_for_times(neuron_in_2.spike_times), marker='|')
    ax[2].scatter(neuron_hidden_1.spike_times, get_spikes_for_times(neuron_hidden_1.spike_times), marker='|')
    ax[3].scatter(neuron_hidden_2.spike_times, get_spikes_for_times(neuron_hidden_2.spike_times), marker='|')
    ax[4].scatter(neuron_output_xor.spike_times, get_spikes_for_times(neuron_output_xor.spike_times), marker='|')
    plt.show()

if __name__ == '__main__':
    #test_behaviour()
    test_xor()