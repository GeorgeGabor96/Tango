package main

import (
	"errors"
	"fmt"
	"log"
	"os"
	"strings"
	"sync"
	"tango/go/network"
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
	meta, _ := network.BuildMeta(args.binFolder)

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

func CreateActivityPlot(meta *network.Meta, sampleName string) {
	data, err := network.BuildData(meta, sampleName)
	if err != nil {
		log.Fatal(err)
	}
	network.ActivityPlot(meta, data, "")
}
