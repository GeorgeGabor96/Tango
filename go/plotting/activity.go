package plotting

import (
	"fmt"
	"tango/go/experiment"
	"tango/go/utils"

	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
	"gonum.org/v1/plot/vg/draw"
)

type ActivityYTicks struct {
	layerTicks []plot.Tick
}

func (t ActivityYTicks) Ticks(min, max float64) []plot.Tick {
	return t.layerTicks
}

type ActivityXTicks struct {
	duration uint32
	nTicks   uint32
}

func (t ActivityXTicks) Ticks(min, max float64) []plot.Tick {
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

func ActivityPlot(meta *experiment.Meta, data *experiment.Data) error {
	fmt.Printf("[INFO] Begin ActivityPlot for %v\n", data.Name)

	sampleName := utils.RemoveExtension(data.Name)
	outFolder := utils.Join(meta.Folder, sampleName)
	imgName := sampleName + "_aux.png"
	imgPath := utils.Join(outFolder, imgName)
	var yPad uint32 = 5

	p := plot.New()

	// NOTE: yPad is space between the line and the neurons so that they do not intersect
	var yOffset uint32 = yPad + 1
	var neuronAbsI uint32 = 0
	var neuronI uint32 = 0
	var stepI uint32 = 0

	// NOTE: for the layer that ends up at the bottom it needs a line below its neurons. Normally the X axis could do it but its ugly
	linePts := make(plotter.XYs, 2)
	linePts[0].X = float64(0)
	linePts[0].Y = float64(yOffset)
	linePts[1].X = float64(data.Duration)
	linePts[1].Y = float64(yOffset)

	l, err := plotter.NewLine(linePts)
	if err != nil {
		return err
	}
	p.Add(l)
	yOffset += yPad
	ticks := make([]plot.Tick, meta.NLayers)

	for i := int(meta.NLayers - 1); i >= 0; i-- {
		layerMeta := meta.Layers[i]
		layerData := data.Layers[i]

		// build the y tick
		ticks[i].Value = float64(yOffset + layerMeta.NNeurons/2)
		ticks[i].Label = layerMeta.Name

		// add the points
		spikePts := make(plotter.XYs, layerData.NSpikes)
		spikePtI := 0

		for stepI = 0; stepI < data.Duration; stepI++ {
			for neuronAbsI, neuronI = layerMeta.NeuronStartIdx, 0; neuronI < layerMeta.NNeurons; neuronAbsI, neuronI = neuronAbsI+1, neuronI+1 {

				neuron := data.Neurons[stepI][neuronAbsI]
				if neuron.Spike {
					spikePts[spikePtI].X = float64(stepI)
					spikePts[spikePtI].Y = float64(yOffset + neuronI)
					spikePtI++
				}
			}
		}

		s, err := plotter.NewScatter(spikePts)
		if err != nil {
			return err
		}
		s.GlyphStyle.Shape = draw.BoxGlyph{}
		s.GlyphStyle.Radius = 1
		p.Add(s)

		// add the layer line
		yOffset += layerMeta.NNeurons + yPad + 1
		linePts := make(plotter.XYs, 2)
		linePts[0].X = float64(0)
		linePts[0].Y = float64(yOffset)
		linePts[1].X = float64(data.Duration)
		linePts[1].Y = float64(yOffset)

		l, err := plotter.NewLine(linePts)
		if err != nil {
			return err
		}
		yOffset += yPad

		p.Add(l)
	}

	// General plot settings
	p.Title.Text = data.Name
	p.Title.TextStyle.Font.Size = PLOTTING_TITLE_FONT_SIZE
	p.X.Label.Text = "time"
	p.X.Label.TextStyle.Font.Size = PLOTTING_LABEL_FONT_SIZE
	p.X.Tick.Marker = &ActivityXTicks{duration: data.Duration, nTicks: 10}
	p.X.Tick.Label.Font.Size = PLOTTING_TICK_FONT_SIZE
	p.Y.Label.Text = "layers"
	p.Y.Label.TextStyle.Font.Size = PLOTTING_LABEL_FONT_SIZE
	p.Y.Tick.Marker = &ActivityYTicks{layerTicks: ticks}
	p.Y.Tick.Label.Font.Size = PLOTTING_TICK_FONT_SIZE

	if err := p.Save(20*vg.Inch, 20*vg.Inch, imgPath); err != nil {
		return err
	}
	fmt.Printf("[INFO] Saved activity plot in %v\n", imgPath)
	return nil
}
