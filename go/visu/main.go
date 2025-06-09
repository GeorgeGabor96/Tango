package main

import (
	"errors"
	"fmt"
	"math/rand"
	"os"
	"strings"
	"sync"
	"tango/go/visu/experiment"
	"tango/go/visu/plotting"
)

type Arguments struct {
	binFolder string
}

func ParseArguments() (*Arguments, error) {
	args := new(Arguments)
	args.binFolder = ""

	for i := 1; i < len(os.Args); i++ {
		if os.Args[i] == "--bin_folder" {
			i++
			args.binFolder = os.Args[i]
		} else {
			return nil, errors.New(fmt.Sprintf("Unknown Argument %v", os.Args[i]))
		}
	}

	if args.binFolder == "" {
		return nil, errors.New(fmt.Sprintf("Must provide the --bin_folder argument"))
	}

	return args, nil
}

func main() {
	args, err := ParseArguments()
	if err != nil {
		panic(err)
	}
	meta, _ := experiment.BuildMeta(args.binFolder)

	accuracyData, err := experiment.BuildAccuracyData(meta)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		plotting.AccuracyPlot(meta, accuracyData)
	}

	var wg sync.WaitGroup
	for sampleI := range meta.Samples {
		var chance float64 = rand.Float64()
		if chance > 0.05 {
			continue
		}

		sample := meta.Samples[sampleI]

		var sampleClone experiment.SampleData
		sampleClone.Name = strings.Clone(sample.Name)
		sampleClone.Duration = sample.Duration
		sampleClone.Epoch = sample.Epoch

		CreatePlots(meta, sampleClone)
		continue
		wg.Add(1)

		go func() {
			CreatePlots(meta, sampleClone)
			wg.Done()
		}()
	}
	wg.Wait()
}

func CreatePlots(meta *experiment.Meta, sample experiment.SampleData) {
	fmt.Printf("[INFO] Begin processing sample %v for epoch %v\n", sample.Name, sample.Epoch)

	data, err := experiment.BuildData(meta, sample)
	if err == nil {
		plotting.SynapseConductancePlot(meta, data, 200)
		plotting.SynapseWeightPlot(meta, data, 200)
		plotting.SynapsesHistPlot(meta, data, 200)

		plotting.NeuronVoltagePlot(meta, data, 200)
		plotting.NeuronSpikesPlot(meta, data, 200)
		plotting.NeuronPscPlot(meta, data, 200)
		plotting.NeuronEpscPlot(meta, data, 200)
		plotting.NeuronIpscPlot(meta, data, 200)
	}

	spikes, err := experiment.BuildSpikes(meta, sample)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		plotting.ActivityPlot(meta, spikes)
		plotting.ActivityPlotForLayer(2, meta, spikes)

		layersI := [2]uint32{3, 4}
		plotting.ActivityPlotForLayers(layersI[:], meta, spikes)
	}

	weights, err := plotting.BuildWeights(meta, sample)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		plotting.PlotWeightHistograms(meta, weights)
	}

	fmt.Printf("[INFO] Finished processing sample %v for epoch %v\n", sample.Name, sample.Epoch)
}
