package plotting

import (
	"errors"
	"fmt"
	"tango/go/utils"

	"gonum.org/v1/plot/plotter"
)

type WeightsHistPlotter struct {
	outFolder string
}

func WeightsHistPlotterCreate(outFolder string) (*WeightsHistPlotter, error) {
	r := new(WeightsHistPlotter)
	utils.CreateFolder(outFolder)
	r.outFolder = outFolder
	return r, nil
}

func (p WeightsHistPlotter) Plot(w *WeightsData) error {
	if w == nil {
		return errors.New("weights is nil")
	}

	nWeights := len(w.Weights[0])
	v := make(plotter.Values, nWeights)
	for step := 0; step < int(w.NSteps); step += 1 {
		p.plotStep(w, v, step)
	}
	return nil
}

func (p WeightsHistPlotter) plotStep(w *WeightsData, v plotter.Values, step int) {
	weights := w.Weights[step]
	for weightI := 0; weightI < len(weights); weightI++ {
		v[weightI] = float64(weights[weightI])
	}

	fileName := fmt.Sprintf("%v_s%v", w.Name, step*int(w.TimeStep))
	HistPlot(v, p.outFolder, fileName)
}
