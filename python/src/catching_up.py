import matplotlib.pyplot as plt

from network.neuron import Neuron
from network.synapse import Synapse
from utils import get_spikes_for_times


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

    neuron_hidden_1 = Neuron() # detects that neuron 1 is firing and neuron 2 is not
    neuron_hidden_2 = Neuron() # detects that neuron 2 is firing and neuron 1 is not

    neuron_in_1.add_out_synapse(synapse_in_1_hidden_1)
    neuron_in_1.add_out_synapse(synapse_in_1_hidden_2)

    neuron_in_2.add_out_synapse(synapse_in_2_hidden_1)
    neuron_in_2.add_out_synapse(synapse_in_2_hidden_2)

    neuron_hidden_1.add_in_synapse(synapse_in_1_hidden_1)
    neuron_hidden_1.add_in_synapse(synapse_in_2_hidden_1)

    neuron_hidden_2.add_in_synapse(synapse_in_1_hidden_2)
    neuron_hidden_2.add_in_synapse(synapse_in_2_hidden_2)

    # hidden layer neg
    neuron_hidden_3 = Neuron() # detects that neuron 1 is firing and neuron 2 is firing
    synapse_in_1_hidden_3 = Synapse(0.5)
    synapse_in_2_hidden_3 = Synapse(0.5)

    neuron_hidden_3.add_in_synapse(synapse_in_1_hidden_3)
    neuron_hidden_3.add_in_synapse(synapse_in_2_hidden_3)

    neuron_in_1.add_out_synapse(synapse_in_1_hidden_3)
    neuron_in_2.add_out_synapse(synapse_in_2_hidden_3)

    neuron_hidden_4 = Neuron() # detects that neuron 1 is not firing and neuron 2 is not firing using neuron_in_3 and neuron_in_4 that spike when in1 and in2 don't
    synapse_in_3_hidden_4 = Synapse(0.5)
    synapse_in_4_hidden_4 = Synapse(0.5)

    neuron_hidden_4.add_in_synapse(synapse_in_3_hidden_4)
    neuron_hidden_4.add_in_synapse(synapse_in_4_hidden_4)

    neuron_in_3.add_out_synapse(synapse_in_3_hidden_4)
    neuron_in_4.add_out_synapse(synapse_in_4_hidden_4)

    # Output Layer
    synapse_hidden_1_output_xor = Synapse(w = 1)
    synapse_hidden_2_output_xor = Synapse(w = 1)
    neuron_output_xor = Neuron()

    neuron_hidden_1.add_out_synapse(synapse_hidden_1_output_xor)
    neuron_hidden_2.add_out_synapse(synapse_hidden_2_output_xor)

    neuron_output_xor.add_in_synapse(synapse_hidden_1_output_xor)
    neuron_output_xor.add_in_synapse(synapse_hidden_2_output_xor)

    synapse_hidden_3_output_nxor = Synapse(w = 1)
    synapse_hidden_4_output_nxor = Synapse(w = 1)
    neuron_output_nxor = Neuron()

    neuron_hidden_3.add_out_synapse(synapse_hidden_3_output_nxor)
    neuron_hidden_4.add_out_synapse(synapse_hidden_4_output_nxor)

    neuron_output_nxor.add_in_synapse(synapse_hidden_3_output_nxor)
    neuron_output_nxor.add_in_synapse(synapse_hidden_4_output_nxor)

    for time in range(100):
        neuron_in_1_inject_current = 1 if (25 <= time < 50 or 75 <= time < 100) and time % 3 == 0 else 0
        neuron_in_1.inject_current(neuron_in_1_inject_current)
        neuron_in_1.update(time)

        neuron_in_2_inject_current = 1 if (50 <= time < 100) and time % 3 == 0 else 0
        neuron_in_2.inject_current(neuron_in_2_inject_current)
        neuron_in_2.update(time)

        neuron_in_3_inject_current = 1 if (time - neuron_in_1.last_spike_time) and time % 3 == 0 else 0
        neuron_in_3.inject_current(neuron_in_3_inject_current)
        neuron_in_3.update(time)

        neuron_in_4_inject_current = 1 if (time - neuron_in_2.last_spike_time) and time % 3 == 0 else 0
        neuron_in_4.inject_current(neuron_in_4_inject_current)
        neuron_in_4.update(time)

        neuron_hidden_1.update(time)
        neuron_hidden_2.update(time)
        neuron_hidden_3.update(time)
        neuron_hidden_4.update(time)

        neuron_output_xor.update(time)
        neuron_output_nxor.update(time)

    _, ax = plt.subplots(10, sharex='all')
    plt.xlim(0, 100)
    ax[0].scatter(neuron_in_1.spike_times, get_spikes_for_times(neuron_in_1.spike_times), marker='|')
    ax[1].scatter(neuron_in_2.spike_times, get_spikes_for_times(neuron_in_2.spike_times), marker='|')
    ax[2].scatter(neuron_in_3.spike_times, get_spikes_for_times(neuron_in_3.spike_times), marker='|')
    ax[3].scatter(neuron_in_4.spike_times, get_spikes_for_times(neuron_in_4.spike_times), marker='|')
    ax[4].scatter(neuron_hidden_1.spike_times, get_spikes_for_times(neuron_hidden_1.spike_times), marker='|')
    ax[5].scatter(neuron_hidden_2.spike_times, get_spikes_for_times(neuron_hidden_2.spike_times), marker='|')
    ax[6].scatter(neuron_hidden_3.spike_times, get_spikes_for_times(neuron_hidden_3.spike_times), marker='|')
    ax[7].scatter(neuron_hidden_4.spike_times, get_spikes_for_times(neuron_hidden_4.spike_times), marker='|')
    ax[8].scatter(neuron_output_xor.spike_times, get_spikes_for_times(neuron_output_xor.spike_times), marker='|')
    ax[9].scatter(neuron_output_nxor.spike_times, get_spikes_for_times(neuron_output_nxor.spike_times), marker='|')
    plt.show()

if __name__ == '__main__':
    #test_behaviour()
    test_xor()