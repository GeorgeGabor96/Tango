from network.network import Network
from network.layer import Layer
from callbacks.spike_plot import SpikePlotCallback
from callbacks.voltage_plot import VoltagePlotCallback
from datagen.random_data_generator import RandomDataGenerator

def main():
    in_layer = Layer(4)
    hidden_layer = Layer(4)
    out_layer = Layer(2)
    net = Network()
    net.add_layer(in_layer)
    net.add_layer(hidden_layer)
    net.add_layer(out_layer)
    net.build()

    callbacks = [
        SpikePlotCallback(),
        VoltagePlotCallback()
    ]

    data = RandomDataGenerator()

    for callback in callbacks:
        callback.before_run(net)

    for time in range(100):
        currents = data.get_currents(net, time)

        net.update(currents, time)

        for callback in callbacks:
            callback.update(net, time)

    for callback in callbacks:
        callback.after_run(net)

    print('success')

if __name__ == '__main__':
    main()

