package plotting

import (
	"errors"
	"fmt"
	"image/color"
	"tango/go/utils"
	"tango/go/visu/experiment"

	"gonum.org/v1/plot/plotter"
)

func NeuronVoltagePlot(meta *experiment.Meta, data *experiment.Data, nI uint32) error {
	if nI >= meta.NNeurons {
		msg := fmt.Sprintf("Invalid neuron index %v - nneurons %v", nI, meta.NNeurons)
		return errors.New(msg)
	}

	outFolder := utils.JoinWithCreate(meta.Folder, utils.RemoveExtension(data.Name))
	outFolder = utils.JoinWithCreate(outFolder, "neurons")
	outFolder = utils.JoinWithCreate(outFolder, "voltage")
	fileName := fmt.Sprintf("%v.png", nI)
	filePath := utils.Join(outFolder, fileName)

	voltagePts := make(plotter.XYs, data.Duration)

	var stepI uint32 = 0
	for stepI = 0; stepI < data.Duration; stepI++ {
		voltagePts[stepI].X = float64(stepI)
		voltagePts[stepI].Y = float64(data.Neurons[stepI][nI].Voltage)
	}

	title := fmt.Sprintf("Neuron %v Voltage evolution", nI)
	err := linePlot(voltagePts, title,
		"Step", IntTicks{nTicks: 10},
		"Voltage", FloatTicks{nTicks: 10},
		color.RGBA{B: 128, A: 255},
		filePath)
	return err
}

func NeuronSpikesPlot(meta *experiment.Meta, data *experiment.Data, nI uint32) error {
	if nI >= meta.NNeurons {
		msg := fmt.Sprintf("Invalid neuron index %v - nneurons %v", nI, meta.NNeurons)
		return errors.New(msg)
	}

	outFolder := utils.JoinWithCreate(meta.Folder, utils.RemoveExtension(data.Name))
	outFolder = utils.JoinWithCreate(outFolder, "neurons")
	outFolder = utils.JoinWithCreate(outFolder, "spikes")
	fileName := fmt.Sprintf("%v.png", nI)
	filePath := utils.Join(outFolder, fileName)

	spikePts := make(plotter.XYs, data.Duration)

	var stepI uint32 = 0
	for stepI = 0; stepI < data.Duration; stepI++ {
		spikePts[stepI].X = float64(stepI)
		if data.Neurons[stepI][nI].Spike {
			spikePts[stepI].Y = 1.0
		} else {
			spikePts[stepI].Y = 0.0
		}
	}

	title := fmt.Sprintf("Neuron %v Spikes evolution", nI)
	err := linePlot(spikePts, title,
		"Step", IntTicks{nTicks: 10},
		"Spikes", IntTicks{nTicks: 2},
		color.RGBA{B: 128, A: 255},
		filePath)
	return err
}

func NeuronPscPlot(meta *experiment.Meta, data *experiment.Data, nI uint32) error {
	if nI >= meta.NNeurons {
		msg := fmt.Sprintf("Invalid neuron index %v - nneurons %v", nI, meta.NNeurons)
		return errors.New(msg)
	}

	outFolder := utils.JoinWithCreate(meta.Folder, utils.RemoveExtension(data.Name))
	outFolder = utils.JoinWithCreate(outFolder, "neurons")
	outFolder = utils.JoinWithCreate(outFolder, "psc")
	fileName := fmt.Sprintf("%v.png", nI)
	filePath := utils.Join(outFolder, fileName)

	pscPts := make(plotter.XYs, data.Duration)

	var stepI uint32 = 0
	for stepI = 0; stepI < data.Duration; stepI++ {
		pscPts[stepI].X = float64(stepI)
		pscPts[stepI].Y = float64(data.Neurons[stepI][nI].PSC)
	}

	title := fmt.Sprintf("Neuron %v PSC evolution", nI)
	err := linePlot(pscPts, title,
		"Step", IntTicks{nTicks: 10},
		"PSC", FloatTicks{nTicks: 10},
		color.RGBA{B: 128, A: 255},
		filePath)
	return err
}

func NeuronEpscPlot(meta *experiment.Meta, data *experiment.Data, nI uint32) error {
	if nI >= meta.NNeurons {
		msg := fmt.Sprintf("Invalid neuron index %v - nneurons %v", nI, meta.NNeurons)
		return errors.New(msg)
	}

	outFolder := utils.JoinWithCreate(meta.Folder, utils.RemoveExtension(data.Name))
	outFolder = utils.JoinWithCreate(outFolder, "neurons")
	outFolder = utils.JoinWithCreate(outFolder, "epsc")
	fileName := fmt.Sprintf("%v.png", nI)
	filePath := utils.Join(outFolder, fileName)

	epscPts := make(plotter.XYs, data.Duration)

	var stepI uint32 = 0
	for stepI = 0; stepI < data.Duration; stepI++ {
		epscPts[stepI].X = float64(stepI)
		epscPts[stepI].Y = float64(data.Neurons[stepI][nI].EPSC)
	}

	title := fmt.Sprintf("Neuron %v EPSC evolution", nI)
	err := linePlot(epscPts, title,
		"Step", IntTicks{nTicks: 10},
		"EPSC", FloatTicks{nTicks: 10},
		color.RGBA{B: 128, A: 255},
		filePath)
	return err
}

func NeuronIpscPlot(meta *experiment.Meta, data *experiment.Data, nI uint32) error {
	if nI >= meta.NNeurons {
		msg := fmt.Sprintf("Invalid neuron index %v - nneurons %v", nI, meta.NNeurons)
		return errors.New(msg)
	}

	outFolder := utils.JoinWithCreate(meta.Folder, utils.RemoveExtension(data.Name))
	outFolder = utils.JoinWithCreate(outFolder, "neurons")
	outFolder = utils.JoinWithCreate(outFolder, "ipsc")
	fileName := fmt.Sprintf("%v.png", nI)
	filePath := utils.Join(outFolder, fileName)

	ipscPts := make(plotter.XYs, data.Duration)

	var stepI uint32 = 0
	for stepI = 0; stepI < data.Duration; stepI++ {
		ipscPts[stepI].X = float64(stepI)
		ipscPts[stepI].Y = float64(data.Neurons[stepI][nI].IPSC)
	}

	title := fmt.Sprintf("Neuron %v IPSC evolution", nI)
	err := linePlot(ipscPts, title,
		"Step", IntTicks{nTicks: 10},
		"IPSC", FloatTicks{nTicks: 10},
		color.RGBA{B: 128, A: 255},
		filePath)
	return err
}
