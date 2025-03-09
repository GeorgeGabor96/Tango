from callbacks.callback import Callback

class NetLearnCallback(Callback):
    def __init__(self):
        pass

    def before_example(self, net):
        self.out_neurons_spike_times = []
        self.max_time = 0
        self.n_out_neurons = len(net.get_out_layer().neurons)

        for i in range(self.n_out_neurons):
            self.out_neurons_spike_times.append([])

    def update(self, net, time):
        neuron_i = 0
        layer = net.get_out_layer()
        for neuron in layer.neurons:
            if neuron.spike:
                self.out_neurons_spike_times[neuron_i].append(time)
            neuron_i += 1
        self.max_time = time

    def after_example(self, net, winners, example_i):
        self.out_neurons_total_spikes = [0 for i in range(len(self.out_neurons_spike_times))]
        for i in range(len(self.out_neurons_spike_times)):
            self.out_neurons_total_spikes[i] = len(self.out_neurons_spike_times[i])

        # get the index with the highes spike count
        winner_i = -1
        winner_score = 0
        for i in range(len(self.out_neurons_total_spikes)):
            if winner_score < self.out_neurons_total_spikes[i]:
                winner_score = self.out_neurons_total_spikes[i]
                winner_i = i

        # for now a single winner
        true_winner = winners[0]

        self.show_weights(net)
        if winner_i != true_winner:
            for i in range(len(self.out_neurons_total_spikes)):

                if i == true_winner:
                    self.increase_weights_on_paths_for_neuron(net, i)
                else:
                    self.decrease_weights_on_paths_for_neruon(net, i)
        self.show_weights(net)

    def increase_weights_on_paths_for_neuron(self, net, i):
        neuron = net.get_out_layer().neurons[i]

        neurons_queue = [neuron]

        while len(neurons_queue) > 0:
            neuron = neurons_queue.pop(0)

            for synapse in neuron.in_synapses:
                synapse.w += 0.1
                synapse.w = min(1, synapse.w) # cap at 1

                neurons_queue.append(synapse.in_neuron)

    def decrease_weights_on_paths_for_neruon(self, net, i):
        neuron = net.get_out_layer().neurons[i]

        neurons_queue = [neuron]

        while len(neurons_queue) > 0:
            neuron = neurons_queue.pop(0)

            for synapse in neuron.in_synapses:
                synapse.w -= 0.1
                synapse.w = max(-1, synapse.w) # cannot go lower than -1

                neurons_queue.append(synapse.in_neuron)

    def show_weights(self, net):
        for layer in net.layers:
            print(layer.name)
            for neuron in layer.neurons:
                for synapse in neuron.in_synapses:
                    print(synapse.w, end=' ')
                print()