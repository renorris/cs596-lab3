#include <WiFi.h>

// Connect to WiFi with a given timeout in milliseconds
wl_status_t connectToWiFi(uint64_t timeoutMs);

// Submit sensor readings to the HTTP server. Returns the HTTP status code.
int submitSensorReading(float temperature, float humidity);
