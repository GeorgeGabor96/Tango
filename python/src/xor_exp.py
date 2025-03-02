from network.network import Network
from network.layer import Layer
from network.weight_init import ZeroWeightInitializer
from callbacks.spike_plot import SpikePlotCallback
from callbacks.voltage_plot import VoltagePlotCallback
from datagen.xor_data_generator import XorDataGenerator

def main():
    in_layer = Layer(4)
    hidden_layer = Layer(4)
    out_layer = Layer(2)
    net = Network()
    net.add_layer(in_layer)
    net.add_layer(hidden_layer)
    net.add_layer(out_layer)
    net.build(weight_initializer=ZeroWeightInitializer())

    callbacks = [
        SpikePlotCallback(),
        #VoltagePlotCallback()
    ]

    data = XorDataGenerator()

    for example_i in range(3):
        example = data.get_example(example_i)
        net.reset()

        for callback in callbacks:
            callback.before_example(net)

        for time in range(100):
            currents = example.get_currents(net, time)

            net.update(currents, time)

            for callback in callbacks:
                callback.update(net, time)

        for callback in callbacks:
            callback.after_example(net)

    print('success')

if __name__ == '__main__':
    main()

