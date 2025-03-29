package main

import (
	"bytes"
	_ "embed"
	"encoding/json"
	"io"
	"log"
	"net/http"
	"os"
	"sync"
)

var username = os.Getenv("CLIENT_USERNAME")
var password = os.Getenv("CLIENT_PASSWORD")

func init() {
	if username == "" {
		log.Fatal("please set CLIENT_USERNAME")
	}
	if password == "" {
		log.Fatal("please set CLIENT_PASSWORD")
	}
}

//go:embed html/index.html
var indexHTML []byte

var temperature float64
var humidity float64
var thLock = sync.RWMutex{}

func main() {
	http.HandleFunc("POST /api/report_sensor_data", func(w http.ResponseWriter, r *http.Request) {
		// Check authorization header
		reqUsername, reqPassword, exists := r.BasicAuth()
		if !exists {
			w.WriteHeader(http.StatusBadRequest)
			return
		}
		if reqUsername != username || reqPassword != password {
			w.WriteHeader(http.StatusForbidden)
			return
		}

		// Read request body into memory
		var bodyBuf bytes.Buffer
		if _, err := io.Copy(&bodyBuf, r.Body); err != nil {
			w.WriteHeader(http.StatusBadRequest)
			return
		}

		// Attempt to parse JSON body
		type RequestBody struct {
			Temperature float64 `json:"temp"`
			Humidity    float64 `json:"humidity"`
		}

		var body RequestBody
		if err := json.Unmarshal(bodyBuf.Bytes(), &body); err != nil {
			w.WriteHeader(http.StatusBadRequest)
			return
		}

		w.WriteHeader(http.StatusNoContent)

		thLock.Lock()
		temperature = body.Temperature
		humidity = body.Humidity
		thLock.Unlock()

		log.Printf("Got sensor report: Temperature = %fC, Humidity = %f%%\n", body.Temperature, body.Humidity)
	})

	// Load basic frontend UI
	http.HandleFunc("GET /", func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Content-Type", "text/html")
		w.WriteHeader(http.StatusOK)
		w.Write(indexHTML)
	})

	// Load sensor data for frontend
	http.HandleFunc("GET /api/get_sensor_data", func(w http.ResponseWriter, r *http.Request) {
		type ResponseBody struct {
			Temperature float64 `json:"temp"`
			Humidity    float64 `json:"humidity"`
		}

		thLock.RLock()
		res := ResponseBody{
			Temperature: temperature,
			Humidity:    humidity,
		}
		thLock.RUnlock()

		jsonRes, err := json.Marshal(&res)
		if err != nil {
			w.WriteHeader(http.StatusInternalServerError)
			return
		}

		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusOK)
		w.Write(jsonRes)
	})

	if err := http.ListenAndServe(":8080", nil); err != nil {
		log.Fatal(err)
	}
}
