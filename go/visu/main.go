package main

import (
	"errors"
	"fmt"
	"log"
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
		fmt.Println(sampleName)
		wg.Add(1)

		go func() {
			CreateActivityPlot(meta, sampleNameCopy)
			wg.Done()
		}()
	}
	wg.Wait()
}

func CreateActivityPlot(meta *experiment.Meta, sampleName string) {
	fmt.Printf("[INFO] Begin processing sample %v\n", sampleName)
	data, err := experiment.BuildData(meta, sampleName)
	if err != nil {
		log.Fatal(err)
	}
	plotting.ActivityPlot(meta, data)

	plotting.SynapseConductancePlot(meta, data, 100)
	plotting.SynapseWeightPlot(meta, data, 100)
	plotting.SynapsesHistPlot(meta, data, 100)

	plotting.NeuronVoltagePlot(meta, data, 100)
	plotting.NeuronSpikesPlot(meta, data, 100)
	plotting.NeuronPscPlot(meta, data, 100)
	plotting.NeuronEpscPlot(meta, data, 100)
	plotting.NeuronIpscPlot(meta, data, 100)

	fmt.Printf("[INFO] Finished processing sample %v\n", sampleName)
}
