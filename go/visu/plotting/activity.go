package plotting

import (
	"errors"
	"fmt"
	"tango/go/utils"
	"tango/go/visu/experiment"

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

func ActivityPlot(meta *experiment.Meta, spikesData *experiment.SpikesData) error {
	layersI := make([]uint32, meta.NLayers)

	var i uint32 = 0
	for i = 0; i < meta.NLayers; i++ {
		layersI[i] = meta.NLayers - 1 - i
	}

	outFolder := utils.JoinWithCreate(meta.PlotsFolder, "activity")

	return activityPlot(layersI, outFolder, meta, spikesData)
}

func ActivityPlotForLayer(i uint32, meta *experiment.Meta, spikesData *experiment.SpikesData) error {
	if i >= meta.NLayers {
		return errors.New(fmt.Sprintf("Layer index %v is invalid. Number of layers is %v\n", i, meta.NLayers))
	}
	layersI := make([]uint32, 1)
	layersI[0] = i

	outFolder := utils.JoinWithCreate(meta.PlotsFolder, fmt.Sprintf("activity_%v", i))

	return activityPlot(layersI, outFolder, meta, spikesData)
}

func activityPlot(layersI []uint32, outFolder string, meta *experiment.Meta, spikesData *experiment.SpikesData) error {
	sampleName := utils.RemoveExtension(spikesData.Name)
	imgName := sampleName + ".png"
	imgPath := utils.Join(outFolder, imgName)
	var yPad uint32 = 5

	p := plot.New()

	// NOTE: yPad is space between the line and the neurons so that they do not intersect
	var yOffset uint32 = yPad + 1

	// NOTE: for the layer that ends up at the bottom it needs a line below its neurons. Normally the X axis could do it but its ugly
	linePts := make(plotter.XYs, 2)
	linePts[0].X = float64(0)
	linePts[0].Y = float64(yOffset)
	linePts[1].X = float64(spikesData.Duration)
	linePts[1].Y = float64(yOffset)

	l, err := plotter.NewLine(linePts)
	if err != nil {
		return err
	}
	p.Add(l)
	yOffset += yPad
	ticks := make([]plot.Tick, meta.NLayers)

	var pair utils.SpikeTimePair
	var pairI uint32 = 0

	for i := 0; i < len(layersI); i++ {
		layerI := layersI[i]
		layerMeta := meta.Layers[layerI]
		layerSpikes := spikesData.Layers[layerI]

		// build the y tick
		ticks[layerI].Value = float64(yOffset + layerMeta.NNeurons/2)
		ticks[layerI].Label = layerMeta.Name

		// add the points
		spikePts := make(plotter.XYs, layerSpikes.NSpikes)

		for pairI = 0; pairI < layerSpikes.NSpikes; pairI++ {
			pair = layerSpikes.Pairs[pairI]
			spikePts[pairI].X = float64(pair.TimeI)
			spikePts[pairI].Y = float64(yOffset + pair.NeuronI)
		}

		s, err := plotter.NewScatter(spikePts)
		if err != nil {
			return err
		}
		s.GlyphStyle.Shape = draw.BoxGlyph{}
		s.GlyphStyle.Radius = 2
		p.Add(s)

		// add the layer line
		yOffset += layerMeta.NNeurons + yPad + 1
		linePts := make(plotter.XYs, 2)
		linePts[0].X = float64(0)
		linePts[0].Y = float64(yOffset)
		linePts[1].X = float64(spikesData.Duration)
		linePts[1].Y = float64(yOffset)

		l, err := plotter.NewLine(linePts)
		if err != nil {
			return err
		}
		yOffset += yPad

		p.Add(l)
	}

	// General plot settings
	p.Title.Text = spikesData.Name
	p.Title.TextStyle.Font.Size = PLOTTING_TITLE_FONT_SIZE
	p.X.Label.Text = "time"
	p.X.Label.TextStyle.Font.Size = PLOTTING_LABEL_FONT_SIZE
	p.X.Tick.Marker = &ActivityXTicks{duration: spikesData.Duration, nTicks: 10}
	p.X.Tick.Label.Font.Size = PLOTTING_TICK_FONT_SIZE
	p.Y.Label.Text = "layers"
	p.Y.Label.TextStyle.Font.Size = PLOTTING_LABEL_FONT_SIZE
	p.Y.Tick.Marker = &ActivityYTicks{layerTicks: ticks}
	p.Y.Tick.Label.Font.Size = PLOTTING_TICK_FONT_SIZE

	if err := p.Save(20*vg.Inch, 20*vg.Inch, imgPath); err != nil {
		return err
	}
	return nil
}
