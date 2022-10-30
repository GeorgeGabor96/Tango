import os
import sys
import argparse
import struct
import numpy as np


def get_args():
	parser = argparse.ArgumentParser()
	parser.add_argument('--bin_file', type=str, required=True)
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
	def __init__(self, sample_duration, n_layers):
		self.sample_duration = sample_duration
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
	net_sample = parse_network_sample_file(args.bin_file)


if __name__ == '__main__':
	main()
