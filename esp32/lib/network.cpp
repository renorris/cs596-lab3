#include <HTTPClient.h>

#include "network.h"
#include "esp_wpa2.h"
#include "credentials.h"

wl_status_t connectToWiFi(uint64_t timeoutMs) {
    #ifdef EAP_ID
    // Configure WPA2 Enterprise credentials
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ID, strlen(EAP_ID));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_USERNAME, strlen(EAP_USERNAME));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
    esp_wifi_sta_wpa2_ent_enable();
    WiFi.begin(WIFI_SSID);
    #else
    // Normal WPA2 Personal
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    #endif

    // Set WiFi to Station mode and disconnect from any previous AP
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    uint64_t beginTime = millis();
    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() > (beginTime + timeoutMs)) {
            Serial.println("timed out");
            return WL_CONNECT_FAILED;
        }
        delay(250);
    }

    Serial.println("Connected to WiFi");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    return WL_CONNECTED;
}

int submitSensorReading(float temperature, float humidity) {
    // Spin up an HTTP client
    HTTPClient client;
    client.begin(REPORT_URL);

    // Set configured basic auth
    client.addHeader("Authorization", BASIC_AUTH);
    client.addHeader("Content-Type", "application/json");

    // Format JSON request body
    char buf[128];
    sprintf(buf, "{\"temp\":%f,\"humidity\":%f}", temperature, humidity);
    String requestBody = String(buf);

    // Do the request
    int responseCode = client.POST(requestBody);

    client.end();

    return responseCode;
}
