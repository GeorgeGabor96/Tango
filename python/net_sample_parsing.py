import sys
import struct
import numpy as np


class SampleFileParser:
	def __init__(self, bin_file: str):
		with open(bin_file, 'rb') as fp:
			bytes = fp.read()
		self.bytes = bytes
		self.offset = 0

	def parse_u32(self) -> int:
		value = int.from_bytes(self.bytes[self.offset: self.offset + 4], byteorder=sys.byteorder, signed=False) 
		self.offset += 4
		return value

	def parse_f32(self) -> float:
		bytes = self.bytes[self.offset: self.offset + 4]
		self.offset += 4
		value = struct.unpack('f', bytes)[0]
		return value

	def parse_bool(self) -> bool:
		return bool(self.parse_u32())

	def parse_string(self) -> str:
		string_len = self.parse_u32()
		string_data = self.bytes[self.offset: self.offset + string_len]
		self.offset += string_len
		string = string_data.decode('utf-8')
		return string


class LayerData:
	def __init__(self, n_steps: int, n_neurons: int):
		self.n_neurons = n_neurons
		self.voltage = np.empty((n_steps, self.n_neurons), dtype=np.float32)
		self.spike = np.empty((n_steps, self.n_neurons), dtype=bool)
		self.psc = np.empty((n_steps, self.n_neurons), dtype=np.float32)
		self.epsc = np.empty((n_steps, self.n_neurons), dtype=np.float32)
		self.ipsc = np.empty((n_steps, self.n_neurons), dtype=np.float32)


class NetworkSample:
	def __init__(self, duration: int, n_layers: int):
		self.duration = duration
		self.n_layers = n_layers
		self.layers_data = []
		self.layers_names = []
		self.layers_y_ticks = []


def parse_network_sample_file(sample_file_path: str) -> NetworkSample:
	print(f'[INFO] Parsing file {sample_file_path}')
	parser = SampleFileParser(sample_file_path)
	sample_duration = parser.parse_u32()
	n_layers = parser.parse_u32()
	net_sample = NetworkSample(sample_duration, n_layers)

	for i in range(n_layers):
		layer_name = parser.parse_string()
		n_neurons = parser.parse_u32()
		layer_data = LayerData(sample_duration, n_neurons)

		for step_i in range(sample_duration):
			
			for neuron_i in range(n_neurons):
				layer_data.voltage[step_i, neuron_i] = parser.parse_f32()
				layer_data.spike[step_i, neuron_i] = parser.parse_bool()
				layer_data.psc[step_i, neuron_i] = parser.parse_f32()
				layer_data.epsc[step_i, neuron_i] = parser.parse_f32()
				layer_data.ipsc[step_i, neuron_i] = parser.parse_f32()

		net_sample.layers_data.insert(0, layer_data)
		net_sample.layers_names.insert(0, layer_name)
		
	y_tick = 1
	for i in range(n_layers):
		layer_data = net_sample.layers_data[i]
	
		layer_y_tick = y_tick + layer_data.n_neurons // 2
		net_sample.layers_y_ticks.append(layer_y_tick)

		y_tick += layer_data.n_neurons + 2

	return net_sample

