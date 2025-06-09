package experiment

import (
	"bufio"
	"errors"
	"fmt"
	"log"
	"os"
	"strconv"
	"tango/go/utils"
	"tango/go/visu/parser"
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

type SampleData struct {
	Name     string
	Duration uint32
	Epoch    uint32
}

type Meta struct {
	Folder      string
	PlotsFolder string
	NLayers     uint32
	NNeurons    uint32
	NSynapses   uint32

	Layers   []LayerMeta
	Synapses []SynapseMeta
	Samples  []SampleData
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

type AccuracyData struct {
	EpochsCount    uint32
	AccuracyValues []float32
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
	meta.PlotsFolder = utils.JoinWithCreate(folder, "plots")
	meta.NLayers = nLayers
	meta.NNeurons = nNeurons
	meta.NSynapses = nSynapses
	meta.Layers = make([]LayerMeta, nLayers)
	meta.Synapses = make([]SynapseMeta, nSynapses)
	meta.Samples = make([]SampleData, 0)

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
		var sample_data SampleData
		sample_data.Name = parser.String()
		sample_data.Duration = parser.Uint32()
		sample_data.Epoch = parser.Uint32()

		meta.Samples = append(meta.Samples, sample_data)
	}

	return meta, nil
}

func BuildData(meta *Meta, sample SampleData) (*Data, error) {
	duration := sample.Duration
	fileName := fmt.Sprintf("data_e%v_%v.bin", sample.Epoch, sample.Name)

	filePath := utils.Join(meta.Folder, fileName)

	parser, err := parser.NewParser(filePath)
	if err != nil {
		return nil, err
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

type SpikesLayer struct {
	NSpikes uint32
	Pairs   []utils.SpikeTimePair
}

type SpikesData struct {
	Name     string
	Duration uint32
	Layers   []SpikesLayer
}

func BuildSpikes(meta *Meta, sample SampleData) (*SpikesData, error) {
	spikesFile := fmt.Sprintf("spikes_e%v_%v.bin", sample.Epoch, sample.Name)
	filePath := utils.Join(meta.Folder, "spikes")
	filePath = utils.Join(filePath, spikesFile)

	parser, err := parser.NewParser(filePath)
	if err != nil {
		return nil, errors.New(fmt.Sprintf("[INFO] Spikes file %v.bin is not present", spikesFile))
	}

	spikesData := new(SpikesData)
	spikesData.Name = spikesFile
	spikesData.Duration = sample.Duration
	spikesData.Layers = make([]SpikesLayer, meta.NLayers)

	var layerI uint32 = 0
	for layerI = 0; layerI < meta.NLayers; layerI++ {
		spikesData.Layers[layerI].NSpikes = 0
		// Start with 1024 pairs capacity, it will realocate more if necessary
		spikesData.Layers[layerI].Pairs = make([]utils.SpikeTimePair, 0, 1024)
	}

	var nTotalSpikes uint32 = parser.Uint32()
	var spikeI uint32 = 0
	var layerForNeuron uint32 = 0
	var pair utils.SpikeTimePair

	for spikeI = 0; spikeI < nTotalSpikes; spikeI++ {
		pair.NeuronI = parser.Uint32()
		pair.TimeI = parser.Uint32()

		// NOTE: find layer for neuron
		for layerI = 0; layerI < meta.NLayers; layerI++ {
			if meta.Layers[layerI].NeuronStartIdx <= pair.NeuronI {
				layerForNeuron = layerI
			} else {
				break
			}
		}
		// NOTE: neurons are offseted to the layer
		pair.NeuronI = pair.NeuronI - meta.Layers[layerForNeuron].NeuronStartIdx

		spikesData.Layers[layerForNeuron].Pairs = append(spikesData.Layers[layerForNeuron].Pairs, pair)
		spikesData.Layers[layerForNeuron].NSpikes++
	}

	return spikesData, nil
}

func BuildAccuracyData(meta *Meta) (*AccuracyData, error) {
	accuracyData := new(AccuracyData)
	accuracyData.EpochsCount = 0
	accuracyData.AccuracyValues = make([]float32, 0)

	accuracyFile := utils.Join(meta.Folder, "accuracy.txt")
	file, err := os.Open(accuracyFile)
	if err != nil {
		return nil, errors.New(fmt.Sprintf("[WARNING] Cannot read accuracy file"))
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	scanner.Split(bufio.ScanLines)

	for scanner.Scan() {
		value, err := strconv.ParseFloat(scanner.Text(), 32)
		if err != nil {
			return nil, errors.New(fmt.Sprintf("[WARNING] Cannot convert str %v to float", value))
		}
		accuracyValue := float32(value)

		accuracyData.EpochsCount += 1
		accuracyData.AccuracyValues = append(accuracyData.AccuracyValues, accuracyValue)
	}

	return accuracyData, nil
}
