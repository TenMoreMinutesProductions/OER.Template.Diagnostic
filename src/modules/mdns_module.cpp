#include "mdns_module.h"
#include <ESPmDNS.h>

static String _hostname;

bool mdnsInit(const char* hostname) {
    _hostname = String(hostname);

    if (MDNS.begin(hostname)) {
        Serial.print("[mDNS] Started: ");
        Serial.print(hostname);
        Serial.println(".local");
        return true;
    } else {
        Serial.println("[mDNS] Failed to start!");
        return false;
    }
}

void mdnsAddService(const char* service, const char* protocol, uint16_t port) {
    MDNS.addService(service, protocol, port);
    Serial.print("[mDNS] Service added: ");
    Serial.print(service);
    Serial.print(".");
    Serial.print(protocol);
    Serial.print(" on port ");
    Serial.println(port);
}

String mdnsGetHostname() {
    return _hostname + ".local";
}
