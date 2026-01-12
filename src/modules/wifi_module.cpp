#include "wifi_module.h"
#include "../config.h"
#include <WiFi.h>

#if USE_ESPNOW
  #include "espnow_module.h"
#endif

static const char* _ssid = nullptr;
static const char* _password = nullptr;
static TaskHandle_t _wifiTaskHandle = nullptr;
static bool _wasConnected = false;

static const unsigned long RECONNECT_INTERVAL = 5000;

// FreeRTOS task running on Core 0
static void wifiTask(void* param) {
    unsigned long lastReconnectAttempt = 0;

    while (true) {
        bool isConnected = (WiFi.status() == WL_CONNECTED);

        if (!isConnected) {
            _wasConnected = false;
            unsigned long now = millis();
            if (now - lastReconnectAttempt >= RECONNECT_INTERVAL) {
                lastReconnectAttempt = now;
                Serial.println("[WiFi] Reconnecting...");
                WiFi.disconnect();
                WiFi.begin(_ssid, _password);
            }
        } else if (!_wasConnected) {
            // Just connected - sync ESP-NOW channel if enabled
            _wasConnected = true;
            Serial.print("[WiFi] Reconnected! IP: ");
            Serial.println(WiFi.localIP());
            #if USE_ESPNOW
              espnowSyncChannel();
            #endif
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

bool wifiInit(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;

    Serial.print("[WiFi] Connecting to ");
    Serial.println(ssid);

    WiFi.setHostname(DEVICE_IDENTIFIER);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    // Wait for connection with timeout (non-blocking after 10s)
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    bool connected = (WiFi.status() == WL_CONNECTED);
    if (connected) {
        Serial.print("[WiFi] Connected! IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("[WiFi] Initial connection failed - will retry in background");
    }

    // Always start WiFi maintenance task on Core 0
    // This ensures reconnection even if initial connection fails
    xTaskCreatePinnedToCore(
        wifiTask,
        "WiFiTask",
        4096,
        NULL,
        1,
        &_wifiTaskHandle,
        0  // Core 0
    );
    Serial.println("[WiFi] Reconnect task started on Core 0");

    return connected;
}

void wifiUpdate() {
    // No longer needed - handled by FreeRTOS task on Core 0
}

bool wifiIsConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String wifiGetIP() {
    return WiFi.localIP().toString();
}

String wifiGetMAC() {
    return WiFi.macAddress();
}
