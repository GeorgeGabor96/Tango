package main

import (
	"errors"
	"fmt"
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

	var wg sync.WaitGroup
	for sampleName := range meta.Samples {
		sampleNameCopy := strings.Clone(sampleName)

		CreatePlots(meta, sampleNameCopy)
		continue
		wg.Add(1)

		go func() {
			CreatePlots(meta, sampleNameCopy)
			wg.Done()
		}()
	}
	wg.Wait()
}

func CreatePlots(meta *experiment.Meta, sampleName string) {
	fmt.Printf("[INFO] Begin processing sample %v\n", sampleName)

	data, err := experiment.BuildData(meta, sampleName)
	if err == nil {
		plotting.SynapseConductancePlot(meta, data, 1000)
		plotting.SynapseWeightPlot(meta, data, 1000)
		plotting.SynapsesHistPlot(meta, data, 1000)

		plotting.NeuronVoltagePlot(meta, data, 1000)
		plotting.NeuronSpikesPlot(meta, data, 1000)
		plotting.NeuronPscPlot(meta, data, 1000)
		plotting.NeuronEpscPlot(meta, data, 1000)
		plotting.NeuronIpscPlot(meta, data, 1000)
	}

	spikes, err := experiment.BuildSpikes(meta, sampleName)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		plotting.ActivityPlot(meta, spikes)
		plotting.ActivityPlotForLayer(2, meta, spikes)

		layersI := [2]uint32{3, 4}
		plotting.ActivityPlotForLayers(layersI[:], meta, spikes)
	}

	weights, err := plotting.BuildWeights(meta, sampleName)
	if err != nil {
		fmt.Println(err.Error())
	} else {
		plotting.PlotWeightHistograms(meta, weights)
	}

	fmt.Printf("[INFO] Finished processing sample %v\n", sampleName)
}
