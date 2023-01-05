package main

import (
	"errors"
	"fmt"
	"log"
	"os"
	"tango/go/network"
	"tango/go/utils"
)

type Arguments struct {
	binFolder string
}

func ParseArguments() (*Arguments, error) {
	args := new(Arguments)

	for i := 1; i < len(os.Args); i++ {
		if os.Args[i] == "--bin_folder" {
			i++
			args.binFolder = os.Args[i]
		} else {
			return nil, errors.New(fmt.Sprintf("Unknown Argument %v", os.Args[i]))
		}
	}

	return args, nil
}

func main() {
	args, err := ParseArguments()
	if err != nil {
		panic(err)
	}
	sampleNames := utils.FileNamesWithExtension(args.binFolder, "bin")
	for _, sampleName := range sampleNames {
		net, err := network.NewNetworkSample(utils.Join(args.binFolder, sampleName))
		if err != nil {
			log.Fatal(err)
		}
		net.ActivityPlot("")
	}
}
