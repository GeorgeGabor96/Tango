package utils

import (
	"image"
	"image/png"
	"log"
	"os"
	"strings"
)

func ReadPngImage(imagePath string) (image.Image, error) {
	f, err := os.Open(imagePath)
	if err != nil {
		log.Fatal(err)
		return nil, err
	}
	defer f.Close()

	img, err := png.Decode(f)
	return img, err
}

func ReadPngImageAsGray(imagePath string) (*image.Gray, error) {
	img, err := ReadPngImage(imagePath)
	if err != nil {
		return nil, err
	}

	// This is kinda stupid because the image may already be grayscale, but whatever
	grayImg := image.NewGray(img.Bounds())
	for y := 0; y < img.Bounds().Max.Y; y++ {
		for x := 0; x < img.Bounds().Max.X; x++ {
			grayImg.Set(x, y, img.At(x, y))
		}
	}

	return grayImg, err
}

func WritePngImage(img image.Image, imgPath string) error {
	if !strings.HasSuffix(imgPath, ".png") {
		imgPath = imgPath + ".png"
	}

	dir := GetFolderFromPath(imgPath)
	CreateFolder(dir)

	fp, err := os.Create(imgPath)
	if err != nil {
		log.Fatal("Couldn't create file", imgPath)
		return err
	}
	defer fp.Close()

	err = png.Encode(fp, img)
	if err != nil {
		log.Fatal("Couldn't encode the png")
		return err
	}
	return nil
}
