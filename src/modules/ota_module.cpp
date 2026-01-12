#include "ota_module.h"
#include <ArduinoOTA.h>

static volatile bool _isUpdating = false;
static String _hostname;
static TaskHandle_t _otaTaskHandle = nullptr;

// FreeRTOS task running on Core 0
static void otaTask(void* param) {
    while (true) {
        ArduinoOTA.handle();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void otaInit(const char* hostname, const char* password) {
    _hostname = String(hostname);

    ArduinoOTA.setHostname(hostname);

    if (password != nullptr && strlen(password) > 0) {
        ArduinoOTA.setPassword(password);
    }

    ArduinoOTA.onStart([]() {
        _isUpdating = true;
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {
            type = "filesystem";
        }
        Serial.println("[OTA] Start updating " + type);
    });

    ArduinoOTA.onEnd([]() {
        _isUpdating = false;
        Serial.println("\n[OTA] Update complete!");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        _isUpdating = false;
        Serial.printf("[OTA] Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();

    // Start OTA task on Core 0
    xTaskCreatePinnedToCore(
        otaTask,
        "OTATask",
        4096,
        NULL,
        1,
        &_otaTaskHandle,
        0  // Core 0
    );

    Serial.print("[OTA] Ready. Hostname: ");
    Serial.println(hostname);
    Serial.println("[OTA] Task started on Core 0");
}

void otaUpdate() {
    // No longer needed - handled by FreeRTOS task on Core 0
}

bool otaIsUpdating() {
    return _isUpdating;
}
