package plotting

import (
	"errors"
	"fmt"
	"math"
	"tango/go/utils"

	"gonum.org/v1/plot/plotter"
)

type WeightsHistPlotter struct {
	outFolder string
	step      int
}

func WeightsHistPlotterCreate(outFolder string, step int) (*WeightsHistPlotter, error) {
	r := new(WeightsHistPlotter)
	utils.CreateFolder(outFolder)
	r.outFolder = outFolder
	if step <= 0 {
		step = math.MaxInt
	}
	r.step = step
	return r, nil
}

func (p WeightsHistPlotter) Plot(w *WeightsData) error {
	if w == nil {
		return errors.New("weights is nil")
	}

	v := make(plotter.Values, len(w.Weights[0]))
	for step := 0; step < int(w.Duration); step += p.step {
		p.plotStep(w, v, step)
	}
	p.plotStep(w, v, int(w.Duration)-1)
	return nil
}

func (p WeightsHistPlotter) plotStep(w *WeightsData, v plotter.Values, step int) {
	weights := w.Weights[step]
	for weightI := 0; weightI < len(weights); weightI++ {
		v[weightI] = float64(weights[weightI])
	}

	fileName := fmt.Sprintf("%v_s%v", w.Name, step)
	HistPlot(v, p.outFolder, fileName)
}
