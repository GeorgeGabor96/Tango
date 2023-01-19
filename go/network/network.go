package network

import (
	"errors"
	"fmt"
	"log"
	"tango/go/parser"
	"tango/go/utils"
)

type LayerMeta struct {
	Name           string
	NeuronStartIdx uint32
	NNeurons       uint32
}

type SynapseMeta struct {
	InNeuronIdx  uint32
	OutNeuronIdx uint32
}

type Meta struct {
	Folder    string
	NLayers   uint32
	NNeurons  uint32
	NSynapses uint32

	Layers   []LayerMeta
	Synapses []SynapseMeta
	Samples  map[string]uint32
}

func BuildMeta(folder string) (*Meta, error) {
	metaFile := utils.Join(folder, "meta.bin")

	fmt.Printf("[INFO] Parsing file %v\n", metaFile)
	parser, err := parser.NewParser(metaFile)
	if err != nil {
		log.Fatal(err)
		return nil, errors.New("Couln't create new parser")
	}

	nLayers := parser.Uint32()
	nNeurons := parser.Uint32()
	nSynapses := parser.Uint32()

	meta := new(Meta)
	meta.Folder = folder
	meta.NLayers = nLayers
	meta.NNeurons = nNeurons
	meta.NSynapses = nSynapses
	meta.Layers = make([]LayerMeta, nLayers)
	meta.Synapses = make([]SynapseMeta, nSynapses)
	meta.Samples = make(map[string]uint32)

	var layerI uint32
	for layerI = 0; layerI < nLayers; layerI++ {
		layer := &(meta.Layers[layerI])
		layer.Name = parser.String()
		layer.NeuronStartIdx = parser.Uint32()
		layer.NNeurons = parser.Uint32()
	}

	// TODO: probably need also some meta for the neurons

	var synapseI uint32
	for synapseI = 0; synapseI < nSynapses; synapseI++ {
		synapse := &(meta.Synapses[synapseI])
		synapse.InNeuronIdx = parser.Uint32()
		synapse.OutNeuronIdx = parser.Uint32()
	}

	for !parser.IsFinished() {
		meta.Samples[parser.String()] = parser.Uint32()
	}

	return meta, nil
}

type NeuronData struct {
	Voltage float32
	Spike   bool
	PSC     float32
	EPSC    float32
	IPSC    float32
}

type SynapseData struct {
	Weight      float32
	Conductance float32
}

type LayerData struct {
	NSpikes uint32
}

type Data struct {
	Name     string
	Duration uint32
	Neurons  [][]NeuronData
	Synapses [][]SynapseData
	Layers   []LayerData
}

func BuildData(meta *Meta, fileName string) (*Data, error) {
	duration, present := meta.Samples[fileName]
	if !present {
		return nil, errors.New("fileName not in meta info")
	}

	filePath := utils.Join(meta.Folder, fileName)

	fmt.Printf("[INFO] Parsing data file %v\n", filePath)
	parser, err := parser.NewParser(filePath)
	if err != nil {
		log.Fatal(err)
		return nil, errors.New("Couldn't create parser for file")
	}

	data := new(Data)
	data.Name = fileName
	data.Duration = duration
	data.Neurons = make([][]NeuronData, duration)
	data.Synapses = make([][]SynapseData, duration)
	data.Layers = make([]LayerData, meta.NLayers)

	var stepI uint32 = 0
	var neuronI uint32 = 0
	var synapseI uint32 = 0
	var layerI uint32 = 0

	for layerI = 0; layerI < meta.NLayers; layerI++ {
		data.Layers[layerI].NSpikes = 0
	}

	for stepI = 0; stepI < duration; stepI++ {
		layerI = 0

		neurons := make([]NeuronData, meta.NNeurons)
		for neuronI = 0; neuronI < meta.NNeurons; neuronI++ {
			neuron := &neurons[neuronI]
			neuron.Voltage = parser.Float32()
			neuron.Spike = parser.Bool()
			neuron.PSC = parser.Float32()
			neuron.EPSC = parser.Float32()
			neuron.IPSC = parser.Float32()

			// NOTE: the neurons are dumped in layer order so this should be valid
			if neuronI >= meta.Layers[layerI].NeuronStartIdx+meta.Layers[layerI].NNeurons {
				layerI++
			}

			if neuron.Spike {
				data.Layers[layerI].NSpikes++
			}
		}

		synapses := make([]SynapseData, meta.NSynapses)
		for synapseI = 0; synapseI < meta.NSynapses; synapseI++ {
			synapse := &synapses[synapseI]
			synapse.Weight = parser.Float32()
			synapse.Conductance = parser.Float32()
		}

		data.Neurons[stepI] = neurons
		data.Synapses[stepI] = synapses
	}

	return data, nil
}
