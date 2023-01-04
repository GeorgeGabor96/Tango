package plotting

import (
	"fmt"
	"image"
	"image/color"
	"image/png"
	"log"
	"os"

	"golang.org/x/image/draw"
	"golang.org/x/image/font"
	"golang.org/x/image/font/basicfont"
	"golang.org/x/image/math/fixed"
)

func BuildImage(nRows uint32, nCols uint32) *image.NRGBA {
	var rect image.Rectangle
	rect.Min.X = 0
	rect.Min.Y = 0
	rect.Max.X = int(nCols)
	rect.Max.Y = int(nRows)
	image := image.NewNRGBA(rect)
	return image
}

func ResizeImage(img *image.NRGBA, newNRows uint32, newNCols uint32) *image.NRGBA {
	dst := BuildImage(newNRows, newNCols)
	draw.NearestNeighbor.Scale(dst, dst.Rect, img, img.Bounds(), draw.Over, nil)
	return dst
}

func SaveImage(path string, img *image.NRGBA) error {
	f, err := os.Create(path)
	if err != nil {
		log.Fatal(err)
		return err
	}
	defer f.Close()

	if err := png.Encode(f, img); err != nil {
		log.Fatal(err)
		return err
	}
	return nil
}

func FillImage(img *image.NRGBA, c color.Color) {
	nRows := img.Rect.Max.Y - img.Rect.Min.Y
	nCols := img.Rect.Max.X - img.Rect.Min.X

	for row := 0; row < nRows; row++ {
		for col := 0; col < nCols; col++ {
			img.Set(col, row, c)
		}
	}
}

func PlotPoint(img *image.NRGBA, row uint32, col uint32, c color.Color) {
	img.Set(int(col), int(row), c)
}

func PlotHLine(img *image.NRGBA, colStart uint32, colEnd uint32, row uint32, c color.Color) {
	for col := colStart; col < colEnd; col++ {
		img.Set(int(col), int(row), c)
	}
}

func PlotVLine(img *image.NRGBA, rowStart uint32, rowEnd uint32, col uint32, c color.Color) {
	for row := rowStart; row < rowEnd; row++ {
		img.Set(int(col), int(row), c)
	}
}

func PlotText(img *image.NRGBA, text string, row uint32, col uint32, c color.Color) {
	point := fixed.Point26_6{fixed.I(int(col)), fixed.I(int(row))}

	d := &font.Drawer{
		Dst:  img,
		Src:  image.NewUniform(c), //color.NRGBA{200, 100, 0, 255}),
		Face: basicfont.Face7x13,
		Dot:  point,
	}
	box, advance := d.BoundString(text)
	fmt.Println(box.Max.Y-box.Min.Y, box.Max.X-box.Min.X, advance)
	d.DrawString(text)
}
