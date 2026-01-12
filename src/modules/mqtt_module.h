#ifndef MQTT_MODULE_H
#define MQTT_MODULE_H

#include <Arduino.h>

// Callback function type for incoming MQTT messages
typedef void (*MqttCallback)(String topic, String payload);

// Initialize MQTT client
// broker: MQTT broker IP/hostname
// port: MQTT broker port (usually 1883)
// deviceId: Device identifier for client name and topics
// topicPrefix: Prefix for topics (e.g., "SP/")
void mqttInit(const char* broker, int port, const char* deviceId, const char* topicPrefix);

// Set callback for incoming messages
void mqttSetCallback(MqttCallback callback);

// Check connection and process messages (call in loop)
void mqttUpdate();

// Publish a message to a topic
// topic: Full topic string or suffix (will be prefixed if not starting with /)
// payload: Message payload
// retained: Whether to retain the message
bool mqttPublish(const char* topic, const char* payload, bool retained = false);

// Subscribe to a topic
bool mqttSubscribe(const char* topic);

// Get connection status
bool mqttIsConnected();

// Get the full base topic (prefix + deviceId)
String mqttGetBaseTopic();

#endif
