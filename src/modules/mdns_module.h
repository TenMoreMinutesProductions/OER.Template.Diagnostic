#ifndef MDNS_MODULE_H
#define MDNS_MODULE_H

#include <Arduino.h>

// Initialize mDNS with hostname
// hostname: Device hostname (will be accessible as hostname.local)
// Returns true if started successfully
bool mdnsInit(const char* hostname);

// Add a service advertisement
// service: Service type (e.g., "http", "mqtt")
// protocol: Protocol (e.g., "tcp", "udp")
// port: Service port
void mdnsAddService(const char* service, const char* protocol, uint16_t port);

// Get the full mDNS hostname
String mdnsGetHostname();

#endif
