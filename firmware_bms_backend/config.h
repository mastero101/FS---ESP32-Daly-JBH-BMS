#ifndef CONFIG_H
#define CONFIG_H

// --- WiFi Configuration ---
const char* ssid     = "Zero02XD 2.4";
const char* password = "ne03252200XD";

// --- Daly/JBD BMS Configuration ---
const char* bms_mac  = "D2:1A:03:01:25:E4";
const uint8_t bms_addr_type = 1; // 0 = PUBLIC (old Daly), 1 = RANDOM (Daly R24TM / JBD)
const char* bms_type = "JBD";    // "DALY" o "JBD"
const char* hostname = "bms-gateway"; // Hostname for mDNS (Access via http://hostname.local)
const unsigned long pollInterval = 30000; // 10 segundos

// --- Backend Configuration ---
const char* backend_url = "http://your-backend-api.com/data";
const unsigned long sendInterval = 30000; // 30 segundos para enviar al backend

#endif
