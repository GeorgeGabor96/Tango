package plotting

import (
	"fmt"
	"image/color"
	"log"
	"tango/go/experiment"
	"tango/go/utils"

	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
)

func SynapseWeightPlot(meta *experiment.Meta, data *experiment.Data, outFolder string, sI uint32) {

}

func SynapseConductancePlot(meta *experiment.Meta, data *experiment.Data, outFolder string, sI uint32) {

}

func LinePlot() {

}

func SynapsesHistPlot(meta *experiment.Meta, data *experiment.Data, outFolder string, stepInc uint32) {
	fmt.Printf("[INFO] Begin Synapse Hist Plots for %v\n", data.Name)

	if outFolder == "" {
		outFolder = meta.Folder
	}

	sampleName := utils.RemoveExtension(data.Name)
	outFolder = utils.Join(outFolder, "synapse_hist")
	utils.CreateFolder(outFolder)
	outFolder = utils.Join(outFolder, sampleName)
	utils.CreateFolder(outFolder)

	weights := make(plotter.Values, meta.NSynapses)

	var stepI uint32 = 0

	for stepI = 0; stepI < data.Duration; stepI += stepInc {

		synapses := data.Synapses[stepI]
		for i, synapse := range synapses {
			weights[i] = float64(synapse.Weight)
		}

		p := plot.New()
		h, err := plotter.NewHist(weights, 16)
		if err != nil {
			log.Panic(err)
		}
		h.FillColor = color.RGBA{B: 128, A: 255}
		p.Add(h)

		title := fmt.Sprintf("Step %v", stepI)
		p.Title.Text = title
		p.Title.TextStyle.Font.Size = 50

		p.X.Label.Text = "weight"
		p.X.Label.TextStyle.Font.Size = 40
		p.X.Tick.Marker = &FloatTicks{nTicks: 10}
		p.X.Tick.Label.Font.Size = 20

		p.Y.Label.Text = "count"
		p.Y.Label.TextStyle.Font.Size = 40
		p.Y.Tick.Marker = &IntTicks{nTicks: 10}
		p.Y.Tick.Label.Font.Size = 20

		imgName := fmt.Sprintf("%v.png", stepI)
		imgPath := utils.Join(outFolder, imgName)

		if err := p.Save(20*vg.Inch, 10*vg.Inch, imgPath); err != nil {
			panic(err)
		}
	}

	fmt.Printf("[INFO] Finished Synapse Hist Plots for %v\n", data.Name)
}
