package parser

import (
	"encoding/binary"
	"errors"
	"log"
	"math"
	"os"
)

type SampleParser struct {
	Bytes  []byte
	Offset uint32
}

func BuildSampleParser(binaryFile string) (*SampleParser, error) {
	bytes, err := os.ReadFile(binaryFile)
	if err != nil {
		log.Fatal(err)
		return nil, errors.New("File doesn't exist")
	}
	parser := new(SampleParser)
	parser.Bytes = bytes
	parser.Offset = 0
	return parser, nil
}

func (parser *SampleParser) Uint32() uint32 {
	value := binary.LittleEndian.Uint32(parser.Bytes[parser.Offset : parser.Offset+4])
	parser.Offset += 4
	return value
}

func (parser *SampleParser) Float32() float32 {
	bits := binary.LittleEndian.Uint32(parser.Bytes[parser.Offset : parser.Offset+4])
	parser.Offset += 4
	value := math.Float32frombits(bits)
	return value
}

func (parser *SampleParser) Bool() bool {
	int_value := parser.Uint32()
	value := false
	if int_value != 0 {
		value = true
	}
	return value
}

func (parser *SampleParser) String() string {
	len := parser.Uint32()
	bytes := parser.Bytes[parser.Offset : parser.Offset+len]
	parser.Offset += len
	str := string(bytes)
	return str
}
