package main

import (
	"errors"
	"fmt"
	"log"
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
		CreateActivityPlot(meta, sampleI)
		continue
		wg.Add(1)

		go func(sampleI int) {
			CreateActivityPlot(meta, sampleI)
			wg.Done()
		}(sampleI)
	}
	wg.Wait()
}

func CreateActivityPlot(meta *experiment.Meta, sampleI int) {
	fmt.Printf("[INFO] Begin processing sample %d\n", sampleI)
	data, err := experiment.BuildData(meta, sampleI)
	if err != nil {
		log.Fatal(err)
	}
	plotting.ActivityPlot(meta, data)

	plotting.SynapseConductancePlot(meta, data, 1000)
	plotting.SynapseWeightPlot(meta, data, 1000)
	plotting.SynapsesHistPlot(meta, data, 1000)

	plotting.NeuronVoltagePlot(meta, data, 1000)
	plotting.NeuronSpikesPlot(meta, data, 1000)
	plotting.NeuronPscPlot(meta, data, 1000)
	plotting.NeuronEpscPlot(meta, data, 1000)
	plotting.NeuronIpscPlot(meta, data, 1000)

	fmt.Printf("[INFO] Finished processing sample %d\n", sampleI)
}
