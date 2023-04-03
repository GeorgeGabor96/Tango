package plotting

import (
	"image/color"

	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
)

const PLOTTING_TITLE_FONT_SIZE = 50
const PLOTTING_LABEL_FONT_SIZE = 40
const PLOTTING_TICK_FONT_SIZE = 20

func linePlot(points plotter.XYs,
	title string,
	xLabel string, xTick plot.Ticker,
	yLabel string, yTick plot.Ticker,
	c color.RGBA,
	filePath string) error {

	p := plot.New()
	l, err := plotter.NewLine(points)
	if err != nil {
		return err
	}
	l.LineStyle.Color = c
	p.Add(l)

	p.Title.Text = title
	p.Title.TextStyle.Font.Size = PLOTTING_TITLE_FONT_SIZE

	p.X.Label.Text = xLabel
	p.X.Label.TextStyle.Font.Size = PLOTTING_LABEL_FONT_SIZE
	p.X.Tick.Marker = xTick
	p.X.Tick.Label.Font.Size = PLOTTING_TICK_FONT_SIZE

	p.Y.Label.Text = yLabel
	p.Y.Label.TextStyle.Font.Size = PLOTTING_LABEL_FONT_SIZE
	p.Y.Tick.Marker = yTick
	p.Y.Tick.Label.Font.Size = PLOTTING_TICK_FONT_SIZE

	if err := p.Save(20*vg.Inch, 10*vg.Inch, filePath); err != nil {
		return err
	}
	return nil
}
