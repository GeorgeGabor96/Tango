package main

import (
	"errors"
	"fmt"
	"os"
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
	for sampleI := 0; sampleI < len(meta.SamplesDuration); sampleI++ {
		CreatePlots(meta, sampleI)
		continue
		wg.Add(1)

		go func(sampleI int) {
			CreatePlots(meta, sampleI)
			wg.Done()
		}(sampleI)
	}
	wg.Wait()
}

func CreatePlots(meta *experiment.Meta, sampleI int) {
	fmt.Printf("[INFO] Begin processing sample %d\n", sampleI)
	data, err := experiment.BuildData(meta, sampleI)
	if err != nil {
		fmt.Println(err)
	} else {
		plotting.SynapseConductancePlot(meta, data, 1000)
		plotting.SynapseWeightPlot(meta, data, 1000)
		plotting.SynapsesHistPlot(meta, data, 1000)

		plotting.NeuronVoltagePlot(meta, data, 1000)
		plotting.NeuronSpikesPlot(meta, data, 1000)
		plotting.NeuronPscPlot(meta, data, 1000)
		plotting.NeuronEpscPlot(meta, data, 1000)
		plotting.NeuronIpscPlot(meta, data, 1000)
	}

	spikes, err := experiment.BuildSpikes(meta, sampleI)
	if err != nil {
		fmt.Printf("[Warning] Cannot build spikes data for sample %d\n", sampleI)
	} else {
		plotting.ActivityPlot(meta, spikes)
	}

	fmt.Printf("[INFO] Finished processing sample %d\n", sampleI)
}
