package main

import (
	"fmt"
	"image"
	"image/color"
	"image/png"
	"log"
	"os"
	"tango/go/utils"
)

var mnistFolder string = "D:/datasets/MNIST"
var outFolder string = "D:/datasets/MNIST/extracted"
var outFolderImg string = utils.Join(outFolder, "img")

func extract(dataFile string, labelFile string, label_name string) {
	labels, err := os.ReadFile(labelFile)
	if err != nil {
		log.Fatal(err)
		return
	}
	labels = labels[8:]
	data_bytes, err := os.ReadFile(dataFile)
	if err != nil {
		log.Fatal(err)
		return
	}
	data_bytes = data_bytes[16:]

	label_file := utils.Join(outFolder, label_name)
	labels_f, _ := os.Create(label_file)

	fmt.Print(label_file)
	for i := 0; i < len(labels); i++ {
		row := fmt.Sprintf("%d\n", labels[i])
		labels_f.WriteString(row)

		bounds := image.Rect(0, 0, 28, 28)
		gray_image := image.NewGray(bounds)

		for row := 0; row < 28; row++ {
			for col := 0; col < 28; col++ {
				gray_image.SetGray(col, row, color.Gray{data_bytes[row*28+col]})
			}
		}
		data_bytes = data_bytes[28*28:]

		img_name := fmt.Sprintf("%d.png", i)
		img_out_path := utils.Join(outFolderImg, img_name)

		out_img_file, _ := os.Create(img_out_path)
		png.Encode(out_img_file, gray_image)
		out_img_file.Close()
	}
	labels_f.Close()

}

func main() {
	utils.CreateFolder(outFolder)
	utils.CreateFolder(outFolderImg)

	train_data_file := utils.Join(mnistFolder, "t10k-images.idx3-ubyte")
	train_label_file := utils.Join(mnistFolder, "t10k-labels.idx1-ubyte")
	extract(train_data_file, train_label_file, "test_label.txt")
}
