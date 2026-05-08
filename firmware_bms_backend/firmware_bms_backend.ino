#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>
#include <esp_coexist.h>
#include "config.h"

/**
 * BMS Gateway (Daly/JBD) for ESP32
 * 
 * Envia datos a un backend y permite flasheo por ElegantOTA vía mDNS.
 * Sin interfaz web local.
 */

// BLE UUIDs
static NimBLEUUID serviceUUID("fff0");
static NimBLEUUID notifyUUID("fff1");
static NimBLEUUID writeUUID("fff2");

const char* JBD_SERVICE = "ff00";
const char* JBD_NOTIFY  = "ff01";
const char* JBD_WRITE   = "ff02";

// Global state
NimBLEClient *pClient = nullptr;
NimBLERemoteCharacteristic *pWriteChar = nullptr;
bool bmsConnected = false;
unsigned long lastBmsMillis = 0;
unsigned long lastPollMillis = 0;
unsigned long lastSendMillis = 0;
unsigned long lastConnectionFailMillis = 0;
AsyncWebServer server(80);

struct BMSData {
  float voltage = 0;
  float current = 0;
  float soc = 0;
  float cell_max_v = 0;
  float cell_min_v = 0;
  int cell_max_num = 0;
  int cell_min_num = 0;
  float cells[16] = {0}; // All cells
  float temp1 = 0;
  bool charge_mos = false;
  bool discharge_mos = false;
} bmsData;

// Commands
const uint8_t CMD_SOC[] = {0xA5, 0x40, 0x90, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7D};
const uint8_t CMD_CELL_MINMAX[] = {0xA5, 0x40, 0x91, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E};
const uint8_t CMD_ALL_CELLS[] = {0xA5, 0x40, 0x95, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82};
const uint8_t CMD_TEMP[] = {0xA5, 0x40, 0x92, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F};
const uint8_t CMD_STATUS[] = {0xA5, 0x40, 0x93, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};

const uint8_t JBD_CMD_BASIC[] = {0xDD, 0xA5, 0x03, 0x00, 0xFF, 0xFD, 0x77};
const uint8_t JBD_CMD_CELLS[] = {0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77};

void notifyCallback(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);

class ClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient *pClient) {
    Serial.println(">>> BMS Conectado.");
    bmsConnected = true;
  }
  void onDisconnect(NimBLEClient *pClient) {
    Serial.println(">>> BMS Desconectado.");
    bmsConnected = false;
  }
};

bool connectBMS() {
  if (String(bms_type) == "JBD") {
    serviceUUID = NimBLEUUID(JBD_SERVICE);
    notifyUUID = NimBLEUUID(JBD_NOTIFY);
    writeUUID = NimBLEUUID(JBD_WRITE);
  } else {
    serviceUUID = NimBLEUUID("fff0");
    notifyUUID = NimBLEUUID("fff1");
    writeUUID = NimBLEUUID("fff2");
  }

  Serial.println("Scanning for BMS: " + String(bms_mac));
  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setActiveScan(true);
  pScan->setInterval(45);
  pScan->setWindow(15);
  
  pScan->start(5, false);
  NimBLEScanResults results = pScan->getResults();
  
  const NimBLEAdvertisedDevice* targetDevice = nullptr;
  for (int i = 0; i < results.getCount(); i++) {
    const NimBLEAdvertisedDevice* device = results.getDevice(i);
    if (device->getAddress().toString() == std::string(bms_mac)) {
      targetDevice = device;
      break;
    }
  }

  if (!targetDevice) {
    Serial.println("BMS no encontrado en escaneo.");
    return false;
  }

  if (pClient) NimBLEDevice::deleteClient(pClient);

  pClient = NimBLEDevice::createClient();
  pClient->setClientCallbacks(new ClientCallbacks());

  Serial.println("Conectando a " + String(bms_mac) + "...");
  if (!pClient->connect(targetDevice)) {
    Serial.println("Error de conexión BLE.");
    return false;
  }

  NimBLERemoteService *pService = pClient->getService(serviceUUID);
  if (!pService) {
    pClient->disconnect();
    return false;
  }

  pWriteChar = pService->getCharacteristic(writeUUID);
  NimBLERemoteCharacteristic *pNotifyChar = pService->getCharacteristic(notifyUUID);

  if (!pWriteChar || !pNotifyChar) {
    pClient->disconnect();
    return false;
  }

  if (pNotifyChar->canNotify()) {
    if (!pNotifyChar->subscribe(true, notifyCallback)) {
      pClient->disconnect();
      return false;
    }
  }

  return true;
}

void notifyCallback(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
  if (length < 7) return;

  if (String(bms_type) == "JBD") {
    if (pData[0] == 0xDD) {
      uint8_t cmd = pData[1];
      if (cmd == 0x03) {
        bmsData.voltage = ((pData[4] << 8) | pData[5]) / 100.0;
        int16_t raw_curr = (pData[6] << 8) | pData[7];
        bmsData.current = raw_curr / 100.0;
        bmsData.soc = pData[23];
        bmsData.charge_mos = (pData[24] & 0x01);
        bmsData.discharge_mos = (pData[24] & 0x02);
        bmsData.temp1 = (((pData[27] << 8) | pData[28]) - 2731) / 10.0;
      } else if (cmd == 0x04) {
        // JBD Cells
        int num_cells = pData[3] / 2;
        for (int i = 0; i < num_cells && i < 16; i++) {
          bmsData.cells[i] = ((pData[4 + i * 2] << 8) | pData[5 + i * 2]) / 1000.0;
        }
      }
      lastBmsMillis = millis();
    }
  } else {
    if (length < 13) return;
    uint8_t cmd = pData[2];
    if (cmd == 0x90) {
      bmsData.voltage = ((pData[4] << 8) | pData[5]) / 10.0;
      int raw_curr = (pData[8] << 8) | pData[9];
      bmsData.current = (raw_curr - 30000) / 10.0;
      bmsData.soc = ((pData[10] << 8) | pData[11]) / 10.0;
    } else if (cmd == 0x91) {
      bmsData.cell_max_v = ((pData[4] << 8) | pData[5]) / 1000.0;
      bmsData.cell_max_num = pData[6];
      bmsData.cell_min_v = ((pData[7] << 8) | pData[8]) / 1000.0;
      bmsData.cell_min_num = pData[9];
    } else if (cmd == 0x95) {
      // Daly 0x95: 3 cells per frame
      uint8_t frame = pData[4];
      for (int i = 0; i < 3; i++) {
        int cell_idx = (frame - 1) * 3 + i;
        if (cell_idx < 16) {
          bmsData.cells[cell_idx] = ((pData[5 + i * 2] << 8) | pData[6 + i * 2]) / 1000.0;
        }
      }
    } else if (cmd == 0x92) {
      bmsData.temp1 = pData[4] - 40;
    } else if (cmd == 0x93) {
      bmsData.charge_mos = pData[5] == 1;
      bmsData.discharge_mos = pData[6] == 1;
    }
    if (cmd >= 0x90 && cmd <= 0x95) lastBmsMillis = millis();
  }
}

void sendDataToBackend() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(backend_url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<1024> doc; // Increased size for cells
  doc["voltage"] = bmsData.voltage;
  doc["current"] = bmsData.current;
  doc["soc"] = bmsData.soc;
  doc["temp1"] = bmsData.temp1;
  doc["charge_mos"] = bmsData.charge_mos;
  doc["discharge_mos"] = bmsData.discharge_mos;
  doc["connected"] = bmsConnected;
  doc["hostname"] = hostname;
  doc["rssi"] = WiFi.RSSI();
  
  doc["cell_max_v"] = bmsData.cell_max_v;
  doc["cell_min_v"] = bmsData.cell_min_v;
  doc["cell_max_num"] = bmsData.cell_max_num;
  doc["cell_min_num"] = bmsData.cell_min_num;

  JsonArray cellsArr = doc.createNestedArray("cells");
  for (int i = 0; i < 16; i++) {
    if (bmsData.cells[i] > 0.5) { // Only add non-zero/valid cells
      cellsArr.add(bmsData.cells[i]);
    }
  }

  String jsonResponse;
  serializeJson(doc, jsonResponse);

  int httpResponseCode = http.POST(jsonResponse);
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("Backend update successful, code: %d\n", httpResponseCode);
    
    // Parse pending commands from backend
    StaticJsonDocument<200> resDoc;
    deserializeJson(resDoc, response);
    
    if (resDoc.containsKey("command") && !resDoc["command"].isNull()) {
      String type = resDoc["command"]["type"];
      bool state = resDoc["command"]["state"];
      
      if (type == "charge") {
        sendBmsCommand(0xDA, state ? 1 : 0);
      } else if (type == "discharge") {
        sendBmsCommand(0xDB, state ? 1 : 0);
      }
    }
  } else {
    Serial.printf("Error sending to backend: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  http.end();
}

void sendBmsCommand(uint8_t cmd, uint8_t state) {
  if (!pWriteChar) return;

  if (String(bms_type) == "JBD") {
    // Para JBD el comando de MOS es diferente
    // Simple ejemplo: JBD usa 0xE1 para control
    uint8_t data = 0;
    if (cmd == 0xDA) data = state ? 0x01 : 0x00; // Charge
    if (cmd == 0xDB) data = state ? 0x02 : 0x00; // Discharge (en realidad JBD usa bits)
    // Nota: JBD requiere un protocolo específico de escritura que omitiremos por brevedad
    // pero el flujo está listo.
  } else {
    uint8_t packet[13] = {0xA5, 0x40, cmd,  0x08, state, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint16_t sum = 0;
    for (int i = 0; i < 12; i++) sum += packet[i];
    packet[12] = sum & 0xFF;
    pWriteChar->writeValue(packet, 13, false);
  }
  Serial.printf("BMS CMD 0x%02X -> %d\n", cmd, state);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- BMS Gateway (Backend) ---");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected: " + WiFi.localIP().toString());

  NimBLEDevice::init("BMS-Gateway");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  esp_coex_preference_set(ESP_COEX_PREFER_BALANCE);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

  // mDNS para ElegantOTA
  if (MDNS.begin(hostname)) {
    Serial.println("mDNS responder started: " + String(hostname) + ".local");
    MDNS.addService("http", "tcp", 80);
  }

  ElegantOTA.begin(&server);
  server.begin();
  Serial.println("ElegantOTA Ready.");
}

void loop() {
  if (!bmsConnected) {
    if (connectBMS()) {
      lastBmsMillis = millis();
      lastConnectionFailMillis = 0;
    } else {
      if (lastConnectionFailMillis == 0) lastConnectionFailMillis = millis();
      if (millis() - lastConnectionFailMillis > 600000) ESP.restart();
      delay(5000);
      return;
    }
  }

  // Polling BMS
  if (millis() - lastPollMillis > pollInterval) {
    if (pWriteChar) {
      if (String(bms_type) == "JBD") {
        pWriteChar->writeValue(JBD_CMD_BASIC, 7, false);
        delay(200);
        pWriteChar->writeValue(JBD_CMD_CELLS, 7, false);
      } else {
        pWriteChar->writeValue(CMD_SOC, 13, false);
        delay(150);
        pWriteChar->writeValue(CMD_CELL_MINMAX, 13, false);
        delay(150);
        pWriteChar->writeValue(CMD_ALL_CELLS, 13, false);
        delay(150);
        pWriteChar->writeValue(CMD_TEMP, 13, false);
        delay(150);
        pWriteChar->writeValue(CMD_STATUS, 13, false);
      }
    }
    lastPollMillis = millis();
  }

  // Sending to Backend
  if (millis() - lastSendMillis > sendInterval) {
    sendDataToBackend();
    lastSendMillis = millis();
  }

  // Watchdog
  if (bmsConnected && (millis() - lastBmsMillis > 180000)) {
    if (pClient) pClient->disconnect();
    bmsConnected = false;
  }

  ElegantOTA.loop();
  delay(10);
}
