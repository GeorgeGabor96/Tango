package plotting

import (
	"fmt"

	"gonum.org/v1/plot"
)

type FloatTicks struct {
	nTicks uint32
}

func (t FloatTicks) Ticks(min, max float64) []plot.Tick {
	ticks := make([]plot.Tick, t.nTicks+1)
	interval := max - min
	tickInc := interval / float64(t.nTicks)

	var i uint32 = 0
	var value float64 = 0.0
	for i = 0; i < t.nTicks+1; i++ {
		value = min + tickInc*float64(i)
		ticks[i].Value = value
		ticks[i].Label = fmt.Sprintf("%.5v", value)
	}
	return ticks
}

type IntTicks struct {
	nTicks uint32
}

func (t IntTicks) Ticks(min, max float64) []plot.Tick {
	ticks := make([]plot.Tick, t.nTicks+1)
	interval := uint32(max - min)
	modValue := interval % t.nTicks
	var tickInc uint32 = 0
	if modValue == 0 {
		tickInc = uint32(interval / t.nTicks)
	} else {
		tickInc = uint32(interval/t.nTicks) + 1
	}

	var i uint32 = 0
	var value float64 = 0.0
	for i = 0; i < t.nTicks; i++ {
		value = min + float64(tickInc*i)
		ticks[i].Value = value
		ticks[i].Label = fmt.Sprint(value)
	}
	value = min + float64(tickInc*i)
	if value > min+float64(interval) {
		value = min + float64(interval)
	}
	ticks[i].Value = float64(value)
	ticks[i].Label = fmt.Sprint(value)
	return ticks
}
