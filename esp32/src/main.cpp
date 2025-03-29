#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT20.h>

#include "esp_wpa2.h"
#include "credentials.h"
#include "network.h"

DHT20 dht20;

void setup() {
  Serial.begin(CONFIG_MONITOR_BAUD);

  // Begin I2C
  Wire.begin();

  // Configure DHT20 sensor
  // After power-on, the sensor needs no less than 100ms stabilization time
  delay(100);
  dht20.begin();
  
  // Connect to WiFi
  wl_status_t wifiStatus = connectToWiFi(60 * 1000);
  if (wifiStatus != WL_CONNECTED) {
      Serial.println("failed to connect to wifi");
      for (;;);
  }
}

void loop() {
    // Read sensor data
    dht20.read();
    float temp = dht20.getTemperature();
    float humidity = dht20.getHumidity();

    // Submit sensor readings
    int httpCode = submitSensorReading(temp, humidity);
    if (httpCode != HTTP_CODE_NO_CONTENT) {
      Serial.printf("[WARN]: HTTP server returned a bad status code: %d\n", httpCode);
    }

    delay(1000);
}
