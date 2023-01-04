package main

import (
	"log"
	"tango/go/network"
)

func main() {
	net, err := network.BuildNetworkSample("d:\\repos\\Tango_outputs\\Threads\\debug\\sample_0.bin")
	if err != nil {
		log.Fatal(err)
	}
	net.ActivityPlot("")
}
