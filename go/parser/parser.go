package parser

import (
	"encoding/binary"
	"errors"
	"log"
	"math"
	"os"
)

type Parser struct {
	Bytes  []byte
	Offset uint32
}

func NewParser(binaryFile string) (*Parser, error) {
	bytes, err := os.ReadFile(binaryFile)
	if err != nil {
		log.Fatal(err)
		return nil, errors.New("File doesn't exist")
	}
	parser := new(Parser)
	parser.Bytes = bytes
	parser.Offset = 0
	return parser, nil
}

func (parser *Parser) Uint32() uint32 {
	value := binary.LittleEndian.Uint32(parser.Bytes[parser.Offset : parser.Offset+4])
	parser.Offset += 4
	return value
}

func (parser *Parser) Float32() float32 {
	bits := binary.LittleEndian.Uint32(parser.Bytes[parser.Offset : parser.Offset+4])
	parser.Offset += 4
	value := math.Float32frombits(bits)
	return value
}

func (parser *Parser) Bool() bool {
	int_value := parser.Uint32()
	value := false
	if int_value != 0 {
		value = true
	}
	return value
}

func (parser *Parser) String() string {
	len := parser.Uint32()
	bytes := parser.Bytes[parser.Offset : parser.Offset+len]
	parser.Offset += len
	str := string(bytes)
	return str
}

func (parser *Parser) IsFinished() bool {
	if parser.Offset >= uint32(len(parser.Bytes)) {
		return true
	}
	return false
}
