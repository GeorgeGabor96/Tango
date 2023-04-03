package plotting

import (
	"errors"
	"fmt"
	"image/color"
	"tango/go/utils"
	"tango/go/visu/experiment"

	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
)

func SynapseWeightPlot(meta *experiment.Meta, data *experiment.Data, sI uint32) error {
	if sI >= meta.NSynapses {
		msg := fmt.Sprintf("Invalid synapse index %v - nsynapses %v", sI, meta.NSynapses)
		return errors.New(msg)
	}

	outFolder := utils.JoinWithCreate(meta.Folder, utils.RemoveExtension(data.Name))
	outFolder = utils.JoinWithCreate(outFolder, "synapses")
	outFolder = utils.JoinWithCreate(outFolder, "weight")
	fileName := fmt.Sprintf("%v.png", sI)
	filePath := utils.Join(outFolder, fileName)

	weightPts := make(plotter.XYs, data.Duration)

	var stepI uint32 = 0
	for stepI = 0; stepI < data.Duration; stepI++ {
		weightPts[stepI].X = float64(stepI)
		weightPts[stepI].Y = float64(data.Synapses[stepI][sI].Weight)
	}

	title := fmt.Sprintf("Synapse %v Weight evolution", sI)
	err := linePlot(weightPts, title,
		"Step", IntTicks{nTicks: 10},
		"Weight", FloatTicks{nTicks: 10},
		color.RGBA{B: 128, A: 255},
		filePath)
	return err
}

func SynapseConductancePlot(meta *experiment.Meta, data *experiment.Data, sI uint32) error {
	if sI >= meta.NSynapses {
		msg := fmt.Sprintf("Invalid synapse index %v - nsynapses %v", sI, meta.Synapses)
		return errors.New(msg)
	}

	outFolder := utils.JoinWithCreate(meta.Folder, utils.RemoveExtension(data.Name))
	outFolder = utils.JoinWithCreate(outFolder, "synapses")
	outFolder = utils.JoinWithCreate(outFolder, "conductance")
	fileName := fmt.Sprintf("%v.png", sI)
	filePath := utils.Join(outFolder, fileName)

	conductancePts := make(plotter.XYs, data.Duration)

	var stepI uint32 = 0
	for stepI = 0; stepI < data.Duration; stepI++ {
		conductancePts[stepI].X = float64(stepI)
		conductancePts[stepI].Y = float64(data.Synapses[stepI][sI].Conductance)
	}

	title := fmt.Sprintf("Synapse %v Conductance evolution", sI)
	err := linePlot(conductancePts, title,
		"Step", IntTicks{nTicks: 10},
		"Conductance", FloatTicks{nTicks: 10},
		color.RGBA{B: 128, A: 255},
		filePath)
	return err
}

func SynapsesHistPlot(meta *experiment.Meta, data *experiment.Data, stepInc uint32) error {
	sampleName := utils.RemoveExtension(data.Name)
	outFolder := utils.JoinWithCreate(meta.Folder, sampleName)
	outFolder = utils.JoinWithCreate(outFolder, "synapse_hist")

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
			return err
		}
		h.FillColor = color.RGBA{B: 128, A: 255}
		p.Add(h)

		title := fmt.Sprintf("Step %v", stepI)
		p.Title.Text = title
		p.Title.TextStyle.Font.Size = PLOTTING_TITLE_FONT_SIZE

		p.X.Label.Text = "weight"
		p.X.Label.TextStyle.Font.Size = PLOTTING_LABEL_FONT_SIZE
		p.X.Tick.Marker = &FloatTicks{nTicks: 10}
		p.X.Tick.Label.Font.Size = PLOTTING_TICK_FONT_SIZE

		p.Y.Label.Text = "count"
		p.Y.Label.TextStyle.Font.Size = PLOTTING_LABEL_FONT_SIZE
		p.Y.Tick.Marker = &IntTicks{nTicks: 10}
		p.Y.Tick.Label.Font.Size = PLOTTING_TICK_FONT_SIZE

		imgName := fmt.Sprintf("%v.png", stepI)
		imgPath := utils.Join(outFolder, imgName)

		if err := p.Save(20*vg.Inch, 10*vg.Inch, imgPath); err != nil {
			return err
		}
	}
	return nil
}
