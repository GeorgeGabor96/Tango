package main

import (
	"log"
	"tango/go/network"
	"tango/go/utils"
)

func main() {
	dir := "d:\\repos\\Tango_outputs\\Threads\\debug\\"
	sampleNames := utils.FileNamesWithExtension(dir, "bin")
	for _, sampleName := range sampleNames {
		net, err := network.NewNetworkSample(utils.Join(dir, sampleName))
		if err != nil {
			log.Fatal(err)
		}
		net.ActivityPlot("")
	}
}
