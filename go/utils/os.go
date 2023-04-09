package utils

import (
	"bufio"
	"errors"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
)

func Join(p1 string, p2 string) string {
	return p1 + string(filepath.Separator) + p2
}

func JoinWithCreate(p1 string, p2 string) string {
	joinFolder := Join(p1, p2)
	CreateFolder(joinFolder)
	return joinFolder
}

func GetFileNamesWithExtension(dir string, ext string) []string {
	var fileNames []string
	items, err := ioutil.ReadDir(dir)
	if err != nil {
		log.Fatal(err)
	}

	for _, item := range items {
		if item.IsDir() == true {
			continue
		}
		itemName := item.Name()
		if GetExtension(itemName) != ext {
			continue
		}
		fileNames = append(fileNames, itemName)
	}
	return fileNames
}

func GetFileNameFromPath(path string) string {
	tokens := strings.Split(path, string(filepath.Separator))
	fileName := tokens[len(tokens)-1]
	return fileName
}

func GetFolderFromPath(path string) string {
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

func GetExtension(fileName string) string {
	tokens := strings.Split(fileName, ".")
	if len(tokens) != 2 {
		log.Fatal(errors.New("Multiple . in filename. Need to extend function"))
	}
	extension := tokens[1]
	return extension
}

func CreateFolder(folder string) bool {
	fi, err := os.Stat(folder)
	if err == nil {
		// If we can read the stat it means something exists just check its a dir
		mode := fi.Mode()
		if mode.IsDir() {
			return true
		}
	}

	if err := os.Mkdir(folder, os.ModePerm); err != nil {
		log.Fatal(err)
		return false
	}
	return true
}

func ReadListingFile(file string) ([]string, error) {
	var items []string

	fp, err := os.Open(file)
	if err != nil {
		log.Fatal(fmt.Sprintf("Couldn't read %s", file))
		return nil, err
	}
	defer fp.Close()

	scanner := bufio.NewScanner(fp)
	// optionally, resize scanner's capacity for lines over 64K, see next example
	for scanner.Scan() {
		items = append(items, scanner.Text())
	}

	if err := scanner.Err(); err != nil {
		log.Fatal(err)
		return nil, err
	}
	return items, nil
}
