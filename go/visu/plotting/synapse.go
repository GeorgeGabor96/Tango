package plotting

import (
	"errors"
	"fmt"
	"image/color"
	"tango/go/utils"
	"tango/go/visu/experiment"
	"tango/go/visu/parser"

	"gonum.org/v1/plot/plotter"
)

type WeightsData struct {
	Name     string
	Duration uint32
	TimeStep uint32
	NSteps   uint32
	Weights  [][]float32 // steps x synapses
}

func BuildWeights(meta *experiment.Meta, sample experiment.SampleData) (*WeightsData, error) {
	fileName := fmt.Sprintf("weights_e%v_%v", sample.Epoch, sample.Name)

	filePath := utils.Join(meta.Folder, "weights")
	filePath = utils.Join(filePath, fileName+".bin")

	parser, err := parser.NewParser(filePath)
	if err != nil {
		return nil, errors.New(fmt.Sprintf("[INFO] Weights file %v.bin is not present", fileName))
	}

	data := new(WeightsData)
	data.Name = fileName
	data.Duration = sample.Duration
	data.TimeStep = parser.Uint32()

	// NOTE: if duration < timestep => nsteps = 0 -> crash
	if sample.Duration < data.TimeStep {
		data.NSteps = 1
	} else {
		data.NSteps = sample.Duration / data.TimeStep
	}

	data.Weights = make([][]float32, data.NSteps)

	var synapseI uint32 = 0
	var stepI uint32 = 0

	for stepI = 0; stepI < data.NSteps; stepI++ {
		weights := make([]float32, meta.NSynapses)
		for synapseI = 0; synapseI < meta.NSynapses; synapseI++ {
			weights[synapseI] = parser.Float32()
		}

		data.Weights[stepI] = weights
	}

	return data, nil
}

func SynapseWeightPlot(meta *experiment.Meta, data *experiment.Data, sI uint32) error {
	if sI >= meta.NSynapses {
		msg := fmt.Sprintf("Invalid synapse index %v - nsynapses %v", sI, meta.NSynapses)
		return errors.New(msg)
	}

	outFolder := utils.JoinWithCreate(meta.PlotsFolder, utils.RemoveExtension(data.Name))
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

	outFolder := utils.JoinWithCreate(meta.PlotsFolder, utils.RemoveExtension(data.Name))
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
	outFolder := utils.JoinWithCreate(meta.PlotsFolder, sampleName)
	outFolder = utils.JoinWithCreate(outFolder, "synapse_hist")

	weights := make(plotter.Values, meta.NSynapses)

	var stepI uint32 = 0

	for stepI = 0; stepI < data.Duration; stepI += stepInc {

		synapses := data.Synapses[stepI]
		for i, synapse := range synapses {
			weights[i] = float64(synapse.Weight)
		}

		err := HistPlot(weights, outFolder, sampleName)
		if err != nil {
			return err
		}
	}
	return nil
}
