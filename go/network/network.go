package network

import (
	"fmt"
)

type NetworkSample struct {
	Duration   uint32
	NLayers    uint32
	Layers     []LayerData
	LayersName []string
}

type LayerData struct {
	NNeurons uint32
	Voltages []float32
	Spikes   []bool
	PSC      []float32
	EPSC     []float32
	IPSC     []float32
}

func BuildNetworkSample(sampleFileType string) *NetworkSample {
	fmt.Printf("[INFO] Parsing file %v", sampleFileType)
	return nil
}
