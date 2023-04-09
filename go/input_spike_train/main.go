package main

import (
	"encoding/binary"
	"errors"
	"fmt"
	"image"
	"log"
	"os"
	"tango/go/utils"
)

type Arguments struct {
	imgFolder   string
	outFolder   string
	listingFile string
}

func ParseArguments() (*Arguments, error) {
	args := new(Arguments)
	args.imgFolder = ""
	args.outFolder = ""
	args.listingFile = ""

	for i := 1; i < len(os.Args); i++ {
		current_arg := os.Args[i]
		switch current_arg {
		case "--img_folder":
			i++
			args.imgFolder = os.Args[i]
		case "--out_folder":
			i++
			args.outFolder = os.Args[i]
		case "--list_file":
			i++
			args.listingFile = os.Args[i]
		default:
			return nil, errors.New(fmt.Sprintf("Unknown Argument %v", current_arg))
		}
	}

	if args.imgFolder == "" {
		return nil, errors.New(fmt.Sprintf("Must provide the --img_folder argument"))
	}
	if args.outFolder == "" {
		return nil, errors.New(fmt.Sprintf("Must provide the --out_folder argument"))
	}
	if args.listingFile == "" {
		return nil, errors.New(fmt.Sprintf("Must provide the --list_file argument"))
	}

	return args, nil
}

func main() {
	args, err := ParseArguments()
	if err != nil {
		log.Fatal(err)
		return
	}

	utils.CreateFolder(args.outFolder)

	fileNames, err := utils.ReadListingFile(args.listingFile)
	if err != nil {
		log.Fatal(err)
		return
	}

	var binNames []string

	for _, imgNamePng := range fileNames {
		imgInPath := utils.Join(args.imgFolder, imgNamePng)
		binName := utils.RemoveExtension(imgNamePng) + ".bin"
		binOutPath := utils.Join(args.outFolder, binName)
		imgGray, err := utils.ReadPngImageAsGray(imgInPath)
		if err != nil {
			log.Fatal(err)
			return
		}

		encodingPairs := EncodeImageWithRoc(imgGray)
		err = WriteSpikePairs(encodingPairs, binOutPath)
		if err != nil {
			log.Fatal(err)
			return
		}
		fmt.Printf("Wrote file %s\n", binOutPath)
		binNames = append(binNames, binName)
	}

	outListFilePath := utils.Join(args.outFolder, "samples.txt")
	fp, err := os.Create(outListFilePath)
	if err != nil {
		log.Fatal(err)
		return
	}
	defer fp.Close()
	for _, binName := range binNames {
		fp.WriteString(binName + "\n")
	}
	fmt.Printf("Wrote sample file %s\n", outListFilePath)
}

type SpikeTimePair struct {
	neuronI uint32
	timeI   uint32
}

type ImgSpikePairs struct {
	encodingType uint32
	nNeurons     uint32
	timeMax      uint32
	width        uint32
	height       uint32
	spikePairs   []SpikeTimePair
}

func EncodeImageWithRoc(imgGray *image.Gray) *ImgSpikePairs {
	imgSpikePairs := new(ImgSpikePairs)
	imgSpikePairs.encodingType = 0
	imgSpikePairs.nNeurons = uint32(imgGray.Bounds().Dx() * imgGray.Bounds().Dy())
	imgSpikePairs.timeMax = 256
	imgSpikePairs.width = uint32(imgGray.Bounds().Dx())
	imgSpikePairs.height = uint32(imgGray.Bounds().Dy())
	spikePairs := make([]SpikeTimePair, imgSpikePairs.nNeurons)

	var i uint32 = 0
	for y := 0; y < imgGray.Bounds().Max.Y; y++ {
		for x := 0; x < imgGray.Bounds().Max.X; x++ {
			spikePairs[i].neuronI = i
			spikePairs[i].timeI = 255 - uint32(imgGray.GrayAt(x, y).Y)
			i++
		}
	}

	imgSpikePairs.spikePairs = spikePairs
	return imgSpikePairs
}

func WriteSpikePairs(spikePairs *ImgSpikePairs, outFile string) error {
	fp, err := os.Create(outFile)
	if err != nil {
		fmt.Printf("Couldn't write %s\n", outFile)
		return err
	}
	defer fp.Close()

	bytes := make([]byte, 4)
	binary.LittleEndian.PutUint32(bytes, spikePairs.timeMax)
	fp.Write(bytes)
	binary.LittleEndian.PutUint32(bytes, spikePairs.nNeurons)
	fp.Write(bytes)

	binary.LittleEndian.PutUint32(bytes, spikePairs.encodingType)
	fp.Write(bytes)
	binary.LittleEndian.PutUint32(bytes, spikePairs.height)
	fp.Write(bytes)
	binary.LittleEndian.PutUint32(bytes, spikePairs.width)
	fp.Write(bytes)

	var n_spike_pairs uint32 = uint32(len(spikePairs.spikePairs))
	binary.LittleEndian.PutUint32(bytes, n_spike_pairs)
	fp.Write(bytes)
	for _, pair := range spikePairs.spikePairs {
		binary.LittleEndian.PutUint32(bytes, pair.neuronI)
		fp.Write(bytes)
		binary.LittleEndian.PutUint32(bytes, pair.timeI)
		fp.Write(bytes)
	}
	return nil
}
