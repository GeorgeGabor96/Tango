package main

import (
	"fmt"
	"image"
	"image/png"
	"log"
	"os"
	"reflect"
)

var imgToEncode string = "d:/datasets/MNIST/extracted/img_train/9999.png"

func read_image(imagePath string) (image.Image, error) {
	f, err := os.Open(imagePath)
	if err != nil {
		log.Fatal(err)
		return nil, err
	}
	defer f.Close()

	image, err := png.Decode(f)
	return image, err
}

func read_image_as_gray(imagePath string) (image.Image, error) {
	image, err := read_image(imagePath)
	if err != nil {
		return nil, err
	}
	image.GrayModel

	return image, err
}

func main() {
	image_path := imgToEncode

	image, err := read_image(image_path)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Print(reflect.TypeOf(image))
}
