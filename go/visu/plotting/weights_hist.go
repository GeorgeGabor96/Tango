package plotting

import (
	"errors"
	"fmt"
	"tango/go/utils"
	"tango/go/visu/experiment"

	"gonum.org/v1/plot/plotter"
)

func PlotWeightHistograms(meta *experiment.Meta, w *WeightsData) error {
	if w == nil {
		return errors.New("weights is nil")
	}

	outFolder := utils.Join(meta.PlotsFolder, "weights_hist")
	nWeights := len(w.Weights[0])
	v := make(plotter.Values, nWeights)
	for step := 0; step < int(w.NSteps); step += 1 {
		plotStep(outFolder, w, v, step)
	}
	return nil
}

func plotStep(outFolder string, w *WeightsData, v plotter.Values, step int) {
	weights := w.Weights[step]
	for weightI := 0; weightI < len(weights); weightI++ {
		v[weightI] = float64(weights[weightI])
	}

	fileName := fmt.Sprintf("%v_s%v", w.Name, step*int(w.TimeStep))
	HistPlot(v, outFolder, fileName)
}
