package main

import (
	"errors"
	"fmt"
	"log"
	"os"
	"sync"
	"tango/go/network"
	"tango/go/utils"
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
	sampleNames := utils.FileNamesWithExtension(args.binFolder, "bin")

	var wg sync.WaitGroup
	for _, sampleName := range sampleNames {
		sampleFile := utils.Join(args.binFolder, sampleName)
		wg.Add(1)

		go func() {
			CreateActivityPlot(sampleFile)
			wg.Done()
		}()
	}
	wg.Wait()
}

func CreateActivityPlot(sampleFile string) {
	net, err := network.NewNetworkSample(sampleFile)
	if err != nil {
		log.Fatal(err)
	}
	net.ActivityPlot("")
}
