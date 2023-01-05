package network

import (
	"errors"
	"fmt"
	"log"
	"tango/go/parser"
)

type NetworkSample struct {
	File     string
	Duration uint32
	NLayers  uint32
	Layers   []LayerData
}

type LayerData struct {
	Name     string
	NNeurons uint32
	NSpikes  uint32
	Voltages []float32
	Spikes   []bool
	PSC      []float32
	EPSC     []float32
	IPSC     []float32
}

func (layer *LayerData) GetIdx(step uint32, neuron uint32) uint32 {
	idx := step*layer.NNeurons + neuron
	return idx
}

func (layer *LayerData) SetVoltage(step uint32, neuron uint32, voltage float32) {
	layer.Voltages[layer.GetIdx(step, neuron)] = voltage
}

func (layer *LayerData) GetVoltage(step uint32, neuron uint32) float32 {
	voltage := layer.Voltages[layer.GetIdx(step, neuron)]
	return voltage
}

func (layer *LayerData) SetSpike(step uint32, neuron uint32, spike bool) {
	layer.Spikes[layer.GetIdx(step, neuron)] = spike
}

func (layer *LayerData) GetSpike(step uint32, neuron uint32) bool {
	spike := layer.Spikes[layer.GetIdx(step, neuron)]
	return spike
}

func (layer *LayerData) SetPSC(step uint32, neuron uint32, psc float32) {
	layer.PSC[layer.GetIdx(step, neuron)] = psc
}

func (layer *LayerData) GetPSC(step uint32, neuron uint32) float32 {
	psc := layer.PSC[layer.GetIdx(step, neuron)]
	return psc
}

func (layer *LayerData) SetEPSC(step uint32, neuron uint32, epsc float32) {
	layer.EPSC[layer.GetIdx(step, neuron)] = epsc
}

func (layer *LayerData) GetEPSC(step uint32, neuron uint32) float32 {
	epsc := layer.EPSC[layer.GetIdx(step, neuron)]
	return epsc
}

func (layer *LayerData) SetIPSC(step uint32, neuron uint32, ipsc float32) {
	layer.IPSC[layer.GetIdx(step, neuron)] = ipsc
}

func (layer *LayerData) GetIPSC(step uint32, neuron uint32) float32 {
	ipsc := layer.IPSC[layer.GetIdx(step, neuron)]
	return ipsc
}

func NewNetworkSample(sampleFile string) (*NetworkSample, error) {
	fmt.Printf("[INFO] Parsing file %v\n", sampleFile)
	netParser, err := parser.BuildSampleParser(sampleFile)
	if err != nil {
		log.Fatal(err)
		return nil, errors.New("Coudn't create net parser")
	}
	sampleDuration := netParser.Uint32()
	nLayers := netParser.Uint32()

	netSample := new(NetworkSample)
	netSample.Duration = sampleDuration
	netSample.NLayers = nLayers
	netSample.Layers = make([]LayerData, nLayers)
	netSample.File = sampleFile

	var layerI uint32
	var stepI uint32
	var neuronI uint32

	for layerI = 0; layerI < nLayers; layerI++ {
		layerName := netParser.String()
		nNeurons := netParser.Uint32()

		layerData := &(netSample.Layers[layerI])
		layerData.Name = layerName
		layerData.NNeurons = nNeurons
		nValues := sampleDuration * nNeurons
		layerData.Voltages = make([]float32, nValues)
		layerData.Spikes = make([]bool, nValues)
		layerData.PSC = make([]float32, nValues)
		layerData.EPSC = make([]float32, nValues)
		layerData.IPSC = make([]float32, nValues)
		var nSpikes uint32 = 0

		for stepI = 0; stepI < sampleDuration; stepI++ {

			for neuronI = 0; neuronI < nNeurons; neuronI++ {
				// NOTE: in the file the values are in this order do not change order of calls for netParser
				layerData.SetVoltage(stepI, neuronI, netParser.Float32())
				spike := netParser.Bool()
				layerData.SetPSC(stepI, neuronI, netParser.Float32())
				layerData.SetEPSC(stepI, neuronI, netParser.Float32())
				layerData.SetIPSC(stepI, neuronI, netParser.Float32())
				if spike == true {
					nSpikes++
				}
				layerData.SetSpike(stepI, neuronI, spike)
			}
		}
		layerData.NSpikes = nSpikes
	}

	fmt.Printf("[INFO] Parsing succesfull for file %v\n", sampleFile)
	return netSample, nil
}
