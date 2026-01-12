#ifndef OTA_MODULE_H
#define OTA_MODULE_H

#include <Arduino.h>

// Initialize OTA updates
// hostname: OTA hostname for identification
// password: OTA password (empty string for no password)
void otaInit(const char* hostname, const char* password = "");

// Handle OTA updates (call in loop)
void otaUpdate();

// Check if OTA update is in progress
bool otaIsUpdating();

#endif
