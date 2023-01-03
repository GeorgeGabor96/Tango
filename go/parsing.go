package tango_parsing

import (
	"encoding/binary"
	"log"
	"math"
	"os"
)

type SampleParser struct {
	Bytes  []byte
	Offset uint32
}

func BuildSampleParser(binaryFile string) *SampleParser {
	bytes, err := os.ReadFile(binaryFile)
	if err != nil {
		log.Fatal(err)
	}
	parser := new(SampleParser)
	parser.Bytes = bytes
	parser.Offset = 0
	return parser
}

func (parser *SampleParser) ReadUint32() uint32 {
	value := binary.LittleEndian.Uint32(parser.Bytes[parser.Offset : parser.Offset+4])
	parser.Offset += 4
	return value
}

func (parser *SampleParser) ReadFloat32() float32 {
	bits := binary.LittleEndian.Uint32(parser.Bytes[parser.Offset : parser.Offset+4])
	parser.Offset += 4
	value := math.Float32frombits(bits)
	return value
}

func (parser *SampleParser) ReadBool() bool {
	int_value := parser.ReadUint32()
	value := false
	if int_value != 0 {
		value = true
	}
	return value
}

func (parser *SampleParser) ReadString() string {
	len := parser.ReadUint32()
	bytes := parser.Bytes[parser.Offset : parser.Offset+len]
	parser.Offset += len
	str := string(bytes)
	return str
}
