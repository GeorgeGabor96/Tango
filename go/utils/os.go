package utils

import (
	"errors"
	"log"
	"path/filepath"
	"strings"
)

func Join(p1 string, p2 string) string {
	return p1 + string(filepath.Separator) + p2
}

func FileNameFromPath(path string) string {
	tokens := strings.Split(path, string(filepath.Separator))
	fileName := tokens[len(tokens)-1]
	return fileName
}

func FolderFromPath(path string) string {
	tokens := strings.Split(path, string(filepath.Separator))
	folder := tokens[0]
	for i := 1; i < len(tokens)-1; i++ {
		folder = Join(folder, tokens[i])
	}
	return folder
}

func RemoveExtension(fileName string) string {
	tokens := strings.Split(fileName, ".")
	if len(tokens) != 2 {
		log.Fatal(errors.New("Multiple . in filename. Need to extend function"))
	}
	fileName = tokens[0]
	return fileName
}
