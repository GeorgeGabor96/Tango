import os
import argparse


def get_args():
	parser = argparse.ArgumentParser()
	parser.add_argument('--bin_file', type=str, required=True)
	return parse.parse_args()


class NeuronStep:
	// TODO: Should I keep them directly in 5 numpy arrays to be fast or no?


class LayerData:
	n_neurons: int
	steps: List[NeuronStep]


class NetworkSample:
	sample_duration: int
	n_layers: int
	layers: List[LayerData]


def parse_network_sample_file(sample_file_path):
	return 0




def main():
	args = get_args()


if __name__ == '__main__':
	main()
