#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <Arduino.h>

// Initialize WiFi connection
// Returns true if connected successfully
bool wifiInit(const char* ssid, const char* password);

// Check and maintain WiFi connection (call in loop)
void wifiUpdate();

// Get connection status
bool wifiIsConnected();

// Get IP address as string
String wifiGetIP();

// Get MAC address as string
String wifiGetMAC();

#endif
