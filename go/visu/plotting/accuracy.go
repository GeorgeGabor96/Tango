package plotting

import (
	"image/color"
	"tango/go/utils"
	"tango/go/visu/experiment"

	"gonum.org/v1/plot/plotter"
)

func AccuracyPlot(meta *experiment.Meta, data *experiment.AccuracyData) error {
	filePath := utils.Join(meta.PlotsFolder, "accuracy.png")

	accuracyPts := make(plotter.XYs, data.EpochsCount)
	var stepI uint32 = 0
	for stepI = 0; stepI < data.EpochsCount; stepI++ {
		accuracyPts[stepI].X = float64(stepI)
		accuracyPts[stepI].Y = float64(data.AccuracyValues[stepI])
	}

	err := linePlot(accuracyPts,
		"Accuracy",
		"Epoch", IntTicks{nTicks: 10},
		"Accuracy", FloatTicks{nTicks: 10},
		color.RGBA{B: 128, A: 255},
		filePath)
	return err
}
