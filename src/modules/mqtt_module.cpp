#include "mqtt_module.h"
#include <WiFi.h>
#include <PubSubClient.h>

static WiFiClient _wifiClient;
static PubSubClient _mqttClient(_wifiClient);
static MqttCallback _userCallback = nullptr;

static String _deviceId;
static String _topicPrefix;
static String _baseTopic;
static String _clientName;
static const char* _broker;
static int _port;

static unsigned long _lastReconnectAttempt = 0;
static const unsigned long RECONNECT_INTERVAL = 5000;

// Internal callback that wraps user callback
static void _internalCallback(char* topic, byte* payload, unsigned int length) {
    if (_userCallback != nullptr) {
        String topicStr = String(topic);
        String payloadStr;
        for (unsigned int i = 0; i < length; i++) {
            payloadStr += (char)payload[i];
        }
        _userCallback(topicStr, payloadStr);
    }
}

static bool _reconnect() {
    if (!_mqttClient.connected()) {
        Serial.print("[MQTT] Connecting as ");
        Serial.print(_clientName);
        Serial.print("... ");

        String statusTopic = _baseTopic + "/status";
        String onlineMsg = _deviceId + " Online";
        String lwtMsg = _deviceId + " Offline (LWT)";

        // LWT (Last Will Testament): Broker publishes lwtMsg if we disconnect unexpectedly
        if (_mqttClient.connect(_clientName.c_str(), statusTopic.c_str(), 0, true, lwtMsg.c_str())) {
            Serial.println("connected!");
            // Publish online status (retained)
            _mqttClient.publish(statusTopic.c_str(), onlineMsg.c_str(), true);
            // Subscribe to command topic
            String cmdTopic = _baseTopic + "/cmd";
            _mqttClient.subscribe(cmdTopic.c_str());
            Serial.print("[MQTT] Subscribed to: ");
            Serial.println(cmdTopic);
            return true;
        } else {
            Serial.print("failed, rc=");
            Serial.println(_mqttClient.state());
            return false;
        }
    }
    return true;
}

void mqttInit(const char* broker, int port, const char* deviceId, const char* topicPrefix) {
    _broker = broker;
    _port = port;
    _deviceId = String(deviceId);
    _topicPrefix = String(topicPrefix);
    _baseTopic = _topicPrefix + _deviceId;
    _clientName = "client_" + _deviceId;

    _mqttClient.setServer(broker, port);
    _mqttClient.setCallback(_internalCallback);

    Serial.print("[MQTT] Broker: ");
    Serial.print(broker);
    Serial.print(":");
    Serial.println(port);
    Serial.print("[MQTT] Base topic: ");
    Serial.println(_baseTopic);

    _reconnect();
}

void mqttSetCallback(MqttCallback callback) {
    _userCallback = callback;
}

void mqttUpdate() {
    if (!_mqttClient.connected()) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt >= RECONNECT_INTERVAL) {
            _lastReconnectAttempt = now;
            _reconnect();
        }
    } else {
        _mqttClient.loop();
    }
}

bool mqttPublish(const char* topic, const char* payload, bool retained) {
    String fullTopic;
    if (topic[0] == '/') {
        fullTopic = String(topic);
    } else {
        fullTopic = _baseTopic + "/" + String(topic);
    }
    return _mqttClient.publish(fullTopic.c_str(), payload, retained);
}

bool mqttSubscribe(const char* topic) {
    String fullTopic;
    if (topic[0] == '/') {
        fullTopic = String(topic);
    } else {
        fullTopic = _baseTopic + "/" + String(topic);
    }
    return _mqttClient.subscribe(fullTopic.c_str());
}

bool mqttIsConnected() {
    return _mqttClient.connected();
}

String mqttGetBaseTopic() {
    return _baseTopic;
}
