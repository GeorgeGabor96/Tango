package network

import (
	"fmt"
	"image/color"
	"log"
	"tango/go/plotting"
	"tango/go/utils"
)

func (netSample *NetworkSample) ActivityPlot(outFolder string) {
	fileName := utils.FileNameFromPath(netSample.File)

	if outFolder == "" {
		outFolder = utils.FolderFromPath(netSample.File)
	}
	imgName := utils.RemoveExtension(fileName) + "_aux.png"

	imgPath := utils.Join(outFolder, imgName)

	var rowPadding uint32 = 20
	var colPadding uint32 = 20

	// Find the dimension of the image and build it
	var i uint32
	var nRows uint32
	var nCols uint32
	var xAxisThickness uint32 = 1
	var yAxisThickness uint32 = 1
	var lineThickness uint32 = 1

	nRows = rowPadding
	for i = 0; i < netSample.NLayers; i++ {
		nRows += netSample.Layers[i].NNeurons + lineThickness
	}
	nRows += xAxisThickness + rowPadding

	nCols = colPadding + yAxisThickness + netSample.Duration + colPadding
	fmt.Println(nRows, nCols)
	img := plotting.BuildImage(nRows, nCols)

	plotting.FillImage(img, color.NRGBA{R: 255, G: 0, B: 0, A: 255})

	// add the axis
	plotting.PlotHLine(img, colPadding, nCols-colPadding, nRows-rowPadding-1, color.Black)
	plotting.PlotVLine(img, rowPadding, nRows-rowPadding, colPadding, color.Black)

	// add the lines on top of the layers
	var lineRow uint32 = rowPadding
	var step uint32
	var neuron uint32
	var step_offset uint32
	var neuron_offset uint32
	for i := 0; i < len(netSample.Layers); i++ {
		plotting.PlotHLine(img, colPadding+yAxisThickness, colPadding+yAxisThickness+netSample.Duration, lineRow, color.Black)
		layer := netSample.Layers[i]

		step_offset = colPadding + yAxisThickness
		neuron_offset = lineRow + lineThickness

		for step = 0; step < netSample.Duration; step++ {

			for neuron = 0; neuron < layer.NNeurons; neuron++ {

				spike := layer.GetSpike(step, neuron)
				if spike == true {
					plotting.PlotPoint(img, neuron_offset+neuron, step_offset+step, color.NRGBA{R: 0, G: 255, B: 0, A: 255})
				}
			}
		}
		plotting.PlotText(img, layer.Name, lineRow, colPadding, color.NRGBA{R: 0, G: 0, B: 255, A: 255})
		lineRow += lineThickness + netSample.Layers[i].NNeurons

	}

	img = plotting.ResizeImage(img, nRows, nRows)
	if err := plotting.SaveImage(imgPath, img); err != nil {
		log.Fatal(err)
	} else {
		fmt.Printf("[INFO] Wrote file %v\n", imgPath)
	}

	// Build the lines of plot
	// First the x and y Axes

}
