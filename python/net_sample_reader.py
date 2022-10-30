import os
import sys
import argparse
import struct
import numpy as np
import matplotlib.pyplot as plt


def get_args():
	parser = argparse.ArgumentParser()
	parser.add_argument('--bin_folder', type=str, required=True)
	return parser.parse_args()


class SampleFileParser:
	def __init__(self, bin_file):
		with open(bin_file, 'rb') as fp:
			bytes = fp.read()
		self.bytes = bytes
		self.offset = 0

	def parse_u32(self):
		value = int.from_bytes(self.bytes[self.offset: self.offset + 4], byteorder=sys.byteorder, signed=False) 
		self.offset += 4
		return value

	def parse_f32(self):
		bytes = self.bytes[self.offset: self.offset + 4]
		self.offset += 4
		value = struct.unpack('f', bytes)[0]
		return value

	def parse_bool(self):
		return self.parse_u32()


class LayerData:
	def __init__(self, n_steps, n_neurons):
		self.n_steps = n_steps
		self.n_neurons = n_neurons
		self.voltage = np.empty((self.n_steps, self.n_neurons), dtype=np.float32)
		self.spike = np.empty((self.n_steps, self.n_neurons), dtype=bool)
		self.psc = np.empty((self.n_steps, self.n_neurons), dtype=np.float32)
		self.epsc = np.empty((self.n_steps, self.n_neurons), dtype=np.float32)
		self.ipsc = np.empty((self.n_steps, self.n_neurons), dtype=np.float32)


class NetworkSample:
	def __init__(self, duration, n_layers):
		self.duration = duration
		self.n_layers = n_layers
		self.layers_data = []


def parse_network_sample_file(sample_file_path):
	print(f'[INFO] Parsing file {sample_file_path}')
	parser = SampleFileParser(sample_file_path)
	sample_duration = parser.parse_u32()
	n_layers = parser.parse_u32()
	net_sample = NetworkSample(sample_duration, n_layers)

	for i in range(n_layers):
		n_neurons = parser.parse_u32()
		layer_data = LayerData(sample_duration, n_neurons)

		for step_i in range(sample_duration):
			
			for neuron_i in range(n_neurons):
				layer_data.voltage[step_i, neuron_i] = parser.parse_f32()
				layer_data.spike[step_i, neuron_i] = parser.parse_bool()
				layer_data.psc[step_i, neuron_i] = parser.parse_f32()
				layer_data.epsc[step_i, neuron_i] = parser.parse_f32()
				layer_data.ipsc[step_i, neuron_i] = parser.parse_f32()

		net_sample.layers_data.append(layer_data)
	return net_sample


def main():
	args = get_args()
	bin_files = [f for f in os.listdir(args.bin_folder) if f.endswith('bin')]
	bin_files.sort()

	for bin_file in bin_files:
		net_sample = parse_network_sample_file(os.path.join(args.bin_folder, bin_file))

		file_name = bin_file.split('.')[0]
		plot_path = os.path.join(args.bin_folder, file_name + '.png')

		plot_net_sample(net_sample, plot_path)


def plot_net_sample(net_sample, plot_path):
	# First plot the lines that separate the layers
	lines = []
	layer_spikes = []
	line_x = np.array([0, net_sample.duration], dtype=np.uint32)
	line_y = np.array([0, 0], dtype=np.uint32)
	lines.append((line_x, line_y))
	y_coord = 1 

	for layer_data in net_sample.layers_data:
		spikes_x, spikes_y = np.where(layer_data.spike == 1)
		spikes_y += y_coord
		layer_spikes.append((spikes_x, spikes_y))

		y_coord += layer_data.n_neurons + 1
		line_x = np.array([0, net_sample.duration], dtype=np.uint32)
		line_y = np.array([y_coord, y_coord], dtype=np.uint32)
		lines.append((line_x, line_y))

	for line in lines:
		print(line)
		plt.plot(line[0], line[1], color='black')
	
	for spikes in layer_spikes:
		print(123)
		print(len(spikes[0]))
		plt.scatter(spikes[0], spikes[1], color='black') 

	plt.show()


if __name__ == '__main__':
	main()
