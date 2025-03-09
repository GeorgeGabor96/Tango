from network.network import Network
from network.layer import Layer
from network.weight_init import ZeroWeightInitializer, UniformWeightInitializer
from callbacks.spike_plot import SpikePlotCallback
from callbacks.voltage_plot import VoltagePlotCallback
from callbacks.net_learn import NetLearnCallback
from callbacks.stdp import StdpCallback
from callbacks.weight_hist import WeightHistCallback
from datagen.xor_data_generator import XorDataGenerator

def main():
    in_layer = Layer(4, 'in')
    hidden_layer = Layer(4, 'hidden')
    out_layer = Layer(2, 'out')
    net = Network()
    net.add_layer(in_layer)
    net.add_layer(hidden_layer)
    net.add_layer(out_layer)
    net.build(weight_initializer=UniformWeightInitializer())

    callbacks = [
        WeightHistCallback(),
        SpikePlotCallback(),
        StdpCallback(),
        #NetLearnCallback(),
        #VoltagePlotCallback()
    ]

    data = XorDataGenerator()

    for example_i in range(100):
        example = data.get_example(example_i)
        net.reset()

        for callback in callbacks:
            callback.before_example(net)

        for time in range(100):
            currents = example.get_currents(net, time)

            net.update(currents, time)

            for callback in callbacks:
                callback.update(net, time)

        winners = example.get_winners(net, time)

        for callback in callbacks:
            callback.after_example(net, winners, example_i)

    print('success')

if __name__ == '__main__':
    main()

