package network

import (
	"fmt"
	"tango/go/utils"

	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
	"gonum.org/v1/plot/vg/draw"
)

type NetYTicks struct {
	layerTicks []plot.Tick
}

func (t NetYTicks) Ticks(min, max float64) []plot.Tick {
	return t.layerTicks
}

type NetXTicks struct {
	duration uint32
	nTicks   uint32
}

func (t NetXTicks) Ticks(min, max float64) []plot.Tick {
	ticks := make([]plot.Tick, t.nTicks+1)
	modValue := t.duration % t.nTicks
	var tickInc uint32 = 0
	if modValue == 0 {
		tickInc = uint32(t.duration / t.nTicks)
	} else {
		tickInc = uint32(t.duration/t.nTicks) + 1
	}

	var i uint32 = 0
	for i = 0; i < t.nTicks; i++ {
		ticks[i].Value = float64(tickInc * i)
		ticks[i].Label = fmt.Sprint(tickInc * i)
	}
	value := tickInc * i
	if value > t.duration {
		value = t.duration
	}
	ticks[i].Value = float64(value)
	ticks[i].Label = fmt.Sprint(value)
	return ticks
}

func (netSample *NetworkSample) ActivityPlot(outFolder string) {
	fmt.Printf("[INFO] Begin ActivityPlot for %v\n", netSample.File)
	fileName := utils.FileNameFromPath(netSample.File)

	if outFolder == "" {
		outFolder = utils.FolderFromPath(netSample.File)
	}
	imgName := utils.RemoveExtension(fileName) + "_aux.png"

	imgPath := utils.Join(outFolder, imgName)
	var yPad uint32 = 5

	p := plot.New()

	var yOffset uint32 = yPad
	var neuron uint32 = 0
	var step uint32 = 0

	linePts := make(plotter.XYs, 2)
	linePts[0].X = float64(0)
	linePts[0].Y = float64(yOffset)
	linePts[1].X = float64(netSample.Duration)
	linePts[1].Y = float64(yOffset)

	l, err := plotter.NewLine(linePts)
	if err != nil {
		panic(err)
	}
	p.Add(l)
	yOffset += yPad
	ticks := make([]plot.Tick, netSample.NLayers)

	for i := len(netSample.Layers) - 1; i >= 0; i-- { //, layer := range netSample.Layers {
		layer := netSample.Layers[i]

		// build the y tick
		ticks[i].Value = float64(yOffset + layer.NNeurons/2)
		ticks[i].Label = layer.Name

		// add the points
		spikePts := make(plotter.XYs, layer.NSpikes)
		spikePtI := 0

		for step = 0; step < netSample.Duration; step++ {
			for neuron = 0; neuron < layer.NNeurons; neuron++ {
				spike := layer.GetSpike(step, neuron)
				if spike == true {
					spikePts[spikePtI].X = float64(step)
					spikePts[spikePtI].Y = float64(yOffset + neuron)
					spikePtI++
				}
			}
		}
		s, err := plotter.NewScatter(spikePts)
		if err != nil {
			panic(err)
		}
		s.GlyphStyle.Shape = draw.BoxGlyph{}
		s.GlyphStyle.Radius = 1
		p.Add(s)

		// add the layer line
		yOffset += layer.NNeurons + yPad
		linePts := make(plotter.XYs, 2)
		linePts[0].X = float64(0)
		linePts[0].Y = float64(yOffset)
		linePts[1].X = float64(netSample.Duration)
		linePts[1].Y = float64(yOffset)

		l, err := plotter.NewLine(linePts)
		if err != nil {
			panic(err)
		}
		yOffset += yPad

		p.Add(l)
	}

	// General plot settings
	p.Title.Text = fileName
	p.Title.TextStyle.Font.Size = 50
	p.X.Label.Text = "time"
	p.X.Label.TextStyle.Font.Size = 40
	p.X.Tick.Marker = &NetXTicks{duration: netSample.Duration, nTicks: 10}
	p.X.Tick.Label.Font.Size = 20
	p.Y.Label.Text = "layers"
	p.Y.Label.TextStyle.Font.Size = 40
	p.Y.Tick.Marker = &NetYTicks{layerTicks: ticks}
	p.Y.Tick.Label.Font.Size = 20

	if err := p.Save(20*vg.Inch, 20*vg.Inch, imgPath); err != nil {
		panic(err)
	}
	fmt.Printf("[INFO] Saved activity plot in %v\n", imgPath)
}
