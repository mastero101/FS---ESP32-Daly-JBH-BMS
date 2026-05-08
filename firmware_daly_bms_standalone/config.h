#ifndef CONFIG_H
#define CONFIG_H

// --- WiFi Configuration ---
const char* ssid     = "Zero02XD 2.4";
const char* password = "ne03252200XD";

// --- Daly/JBD BMS Configuration ---
const char* bms_mac  = "D2:1A:03:01:25:E4";
const uint8_t bms_addr_type = 1; // 0 = PUBLIC (old Daly), 1 = RANDOM (Daly R24TM / JBD)
const char* bms_type = "JBD";    // "DALY" o "JBD"
const char* hostname = "dalybms";
const unsigned long pollInterval = 10000; // 10 seconds for standalone (faster than bridge)

#endif
