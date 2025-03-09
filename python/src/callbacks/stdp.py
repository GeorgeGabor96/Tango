from callbacks.callback import Callback

import math

class StdpCallback(Callback):
    def __init__(self):
        pass

    def before_example(self, net):
        pass

    def update(self, net, time):

        for layer in net.layers:

            for neuron in layer.neurons:

                # process only input synapses
                for synapse in neuron.in_synapses:
                    self.update_synapse_w(synapse)


    def after_example(self, net, winners, example_i):
        pass


    def update_synapse_w(self, synapse):
        if synapse.w_changed:
            return

        in_neuron = synapse.in_neuron
        out_neuron = synapse.out_neuron

        in_neuron_last_spike_time = in_neuron.last_spike_time
        out_neuron_last_spike_time = out_neuron.last_spike_time

        # only update if both neurons did something
        if in_neuron_last_spike_time < 0:
            return

        if out_neuron_last_spike_time < 0:
            return

        if in_neuron_last_spike_time >= out_neuron_last_spike_time:
            # input neuron spiked ofter output neuron, it didn't contribute to the spike of the output one -> LTD
            dt = in_neuron_last_spike_time - out_neuron_last_spike_time
            dw = - 0.1 * math.exp(dt / 20)
        else:
            # input neuron spiked before output neruon, it did contribute to the output neruon spike -> LTP
            dt = out_neuron_last_spike_time - in_neuron_last_spike_time
            dw = 0.1 * math.exp(dt / 20)

        synapse.w = max(min(synapse.w + dw, 1), -1)
        synapse.w_changed = True
        return

    # TODO: try with full history?