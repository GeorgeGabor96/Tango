from network.network import Network
from network.layer import Layer
from callbacks.spike_plot import SpikePlotCallback
from callbacks.voltage_plot import VoltagePlotCallback
from datagen.random_data_generator import RandomDataGenerator
from network.weight_init import UniformWeightInitializer

def main():
    in_layer = Layer(4)
    hidden_layer = Layer(4)
    out_layer = Layer(2)
    net = Network()
    net.add_layer(in_layer)
    net.add_layer(hidden_layer)
    net.add_layer(out_layer)
    net.build(weight_initializer=UniformWeightInitializer())

    callbacks = [
        SpikePlotCallback(),
        VoltagePlotCallback()
    ]

    data = RandomDataGenerator()

    for example_i in range(10):
        example = data.get_example(example_i)

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

