package main

import (
	"fmt"
	"io"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"github.com/labstack/echo/v4"
)

func main() {

	e := echo.New()

	//*******************POST*******************

	//~~~~~~~~~~~~~~~Upload Image~~~~~~~~~~~~~~~

	e.POST("/upload", func(c echo.Context) error {

		// Read form data
		file, err := c.FormFile("file")

		if err != nil {
			return err
		}

		// Source
		src, err := file.Open()

		if err != nil {
			return c.String(http.StatusOK, "File does not exists\n")
		}
		defer src.Close()

		// Destination directory
		destDir := "/var/www/image-viewer/img/"

		// Ensure destination directory exists
		if err := os.MkdirAll(destDir, os.ModePerm); os.IsNotExist(err) {
			return c.String(http.StatusOK, "Destination directory does not exist\n")
		}

		// Destination
		dstPath := filepath.Join(destDir, file.Filename)
		dst, err := os.Create(dstPath)

		if err != nil {
			return c.String(http.StatusOK, "Couldn't create file\n")
		}
		defer dst.Close()

		// Copy
		if _, err = io.Copy(dst, src); err != nil {
			return c.String(http.StatusOK, "Couldn't copy file\n")
		}
		fmt.Printf("File uploaded: %s\n", dstPath)

		return c.String(http.StatusOK, "File uploaded successfully\n")
	})

	//*******************GET********************

	//~~~~~~~~~~~~~~Download Image~~~~~~~~~~~~~~

	e.GET("/download/:file", func(c echo.Context) error {

		// Read form data
		file := c.Param("file")

		file = "/var/www/image-viewer/img/" + file

		// Ensure file exists
		if _, err := os.Stat(file); os.IsNotExist(err) {
			return c.String(http.StatusOK, "File does not exist\n")
		}

		fmt.Printf("File download: %s\n", file)

		// Send file
		return c.File(file)
	})

	//~~~~~~~~~~~~~~~List Images~~~~~~~~~~~~~~~~

	e.GET("/list", func(c echo.Context) error {

		// Destination directory
		destDir := "/var/www/image-viewer/img/"

		// Ensure destination directory exists
		if err := os.MkdirAll(destDir, os.ModePerm); os.IsNotExist(err) {
			return c.String(http.StatusOK, "Destination directory does not exist\n")
		}

		// Read directory contents
		files, err := os.ReadDir(destDir)
		if err != nil {
			return err
		}

		// Extract image file names
		var imageList []string
		for _, file := range files {
			if !file.IsDir() && strings.HasSuffix(strings.ToLower(file.Name()), ".png") || strings.HasSuffix(strings.ToLower(file.Name()), ".jpg") || strings.HasSuffix(strings.ToLower(file.Name()), ".ppm") {
				imageList = append(imageList, file.Name())
			}
		}

		// Send list of images
		return c.JSON(http.StatusOK, imageList)
	})

	//~~~~~~~~~~~~~~Display Image~~~~~~~~~~~~~~~

	e.GET("/display/:file", func(c echo.Context) error {

		// Read form data
		file := c.Param("file")

		file = "/var/www/image-viewer/img/" + file

		// Ensure file exists
		if _, err := os.Stat(file); os.IsNotExist(err) {
			return c.String(http.StatusOK, "File does not exist\n")
		}

		// Display the image
		command := exec.Command("/var/www/image-viewer/api/display", file)

		if err := command.Start(); err != nil {
			return c.String(http.StatusOK, "Couldn't display image\n")
		}
		fmt.Printf("File displayed: %s\n", file)

		return c.String(http.StatusOK, "Look at the screen\n")
	})

	//******************DELETE******************

	//~~~~~~~~~~~~~~~Delete Image~~~~~~~~~~~~~~~

	e.DELETE("/delete/:file", func(c echo.Context) error {

		// Read form data
		file := c.Param("file")

		file = "/var/www/image-viewer/img/" + file

		// Ensure file exists
		if _, err := os.Stat(file); os.IsNotExist(err) {
			return c.String(http.StatusOK, "File does not exist\n")
		}

		// Delete
		if err := os.Remove(file); err != nil {
			return c.String(http.StatusOK, "Couldn't delete file\n")
		}
		fmt.Printf("File deleted: %s\n", file)

		return c.String(http.StatusOK, "File deleted successfully\n")
	})

	e.Logger.Fatal(e.Start(":1323"))
}
