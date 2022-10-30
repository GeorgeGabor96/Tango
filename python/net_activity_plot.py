import os
import argparse
import matplotlib.pyplot as plt
import numpy as np

from net_sample_parsing import parse_network_sample_file, NetworkSample
from utils import get_names_with_extension, remove_extension


def get_args():
	parser = argparse.ArgumentParser()
	parser.add_argument('--bin_folder', type=str, required=True)
	return parser.parse_args()

def main():
	args = get_args()
	bin_files = get_names_with_extension(args.bin_folder, 'bin')

	for bin_file in bin_files:
		net_sample = parse_network_sample_file(os.path.join(args.bin_folder, bin_file))

		file_name = remove_extension(bin_file)
		plot_path = os.path.join(args.bin_folder, file_name + '.png')

		plot_net_sample(net_sample, plot_path)


def plot_net_sample(net_sample: NetworkSample, plot_path: str):
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

	# Actual plotting
	plt.clf()
	plt.cla()
	plt.close()
	plt.figure(figsize=(10, 10))
	plt.xlabel('time', fontsize=30)
	plt.ylabel('layers', fontsize=30)

	for line in lines:
		plt.plot(line[0], line[1], color='black')
	
	for spikes in layer_spikes:
		plt.scatter(spikes[0], spikes[1], color='black', s=0.5) 

	if plot_path:
		print(f'[INFO] Saving in {plot_path}')
		plt.savefig(plot_path)
	else:
		plt.show()


if __name__ == '__main__':
	main()
