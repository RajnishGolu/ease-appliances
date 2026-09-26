#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>

#define DEVICE_NAME "Ease Appliances"

// 4-Channel Relay Output Pins (Active LOW for optocoupler relay boards)
#define RELAY1_PIN 23
#define RELAY2_PIN 22
#define RELAY3_PIN 21
#define RELAY4_PIN 19
#define TEST_LED_PIN 2

// UUIDs
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_WIFI_PROV_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_STATUS_UUID       "beb5483f-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_RELAY_UUID        "beb54840-36e1-4688-b7f5-ea07361b26a8"

Preferences prefs;
NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pStatusChar = nullptr;
NimBLECharacteristic* pRelayChar = nullptr;
WebServer httpServer(80);

bool deviceConnected = false;
bool oldDeviceConnected = false;
bool httpServerStarted = false;

// 4 Relay States (Active LOW: LOW = Relay energized/ON, HIGH = Relay de-energized/OFF)
bool relay1State = false;
bool relay2State = false;
bool relay3State = false;
bool relay4State = false;

// Persistent Counters & Tracking
uint32_t onCount = 0;
uint32_t offCount = 0;
uint32_t totalToggles = 0;

// Firebase Cloud Sync
const char* DEFAULT_FIREBASE_HOST = "https://smart-plug-c131f-default-rtdb.firebaseio.com";
String firebaseHost = DEFAULT_FIREBASE_HOST;
String firebaseAuth = ""; // optional auth secret
bool internetConnected = false;

String newSsid = "";
String newPass = "";
bool pendingWifiConnect = false;

void syncToFirebase();
void setRelayChannel(int ch, bool state, bool pushToCloud = true);
void unpairAndResetDevice();

void applyRelayPins() {
  // Optocoupler relays are Active LOW:
  // LOW (0V) -> Relay ON (contacts close, indicator LED on board lights up)
  // HIGH (3.3V) -> Relay OFF (contacts open, indicator LED on board turns off)
  digitalWrite(RELAY1_PIN, relay1State ? LOW : HIGH);
  digitalWrite(RELAY2_PIN, relay2State ? LOW : HIGH);
  digitalWrite(RELAY3_PIN, relay3State ? LOW : HIGH);
  digitalWrite(RELAY4_PIN, relay4State ? LOW : HIGH);

  // Onboard blue LED (GPIO 2) illuminates if ANY relay is active
  digitalWrite(TEST_LED_PIN, (relay1State || relay2State || relay3State || relay4State) ? HIGH : LOW);
}

String getFullStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    if (internetConnected) {
      return "CONNECTED:" + WiFi.localIP().toString() + ":" + WiFi.SSID();
    } else {
      return "WIFI_NO_INTERNET:" + WiFi.localIP().toString() + ":" + WiFi.SSID();
    }
  }
  return "READY_FOR_PROVISIONING";
}

void updateStatus(const String& statusMsg) {
  Serial.print("[STATUS] ");
  Serial.println(statusMsg);
  if (pStatusChar) {
    pStatusChar->setValue(statusMsg.c_str());
    if (deviceConnected) {
      pStatusChar->notify();
    }
  }
}

// Push live state & counters to Firebase Realtime Database
void syncToFirebase() {
  if (WiFi.status() != WL_CONNECTED || firebaseHost.length() < 8) return;

  WiFiClientSecure client;
  client.setInsecure();
  client.setHandshakeTimeout(5);
  HTTPClient https;

  String url = firebaseHost;
  if (!url.startsWith("http://") && !url.startsWith("https://")) {
    url = "https://" + url;
  }
  if (!url.endsWith("/")) url += "/";
  url += "ease_appliances/state.json";
  if (firebaseAuth.length() > 0) {
    url += "?auth=" + firebaseAuth;
  }

  if (https.begin(client, url)) {
    https.setTimeout(3500);
    https.addHeader("Content-Type", "application/json");
    JsonDocument doc;
    doc["online"] = true;
    doc["heartbeat"] = millis();
    doc["last_seen"][".sv"] = "timestamp";
    doc["relay"] = (relay1State || relay2State || relay3State || relay4State);

    JsonObject relays = doc["relays"].to<JsonObject>();
    relays["r1"] = relay1State;
    relays["r2"] = relay2State;
    relays["r3"] = relay3State;
    relays["r4"] = relay4State;

    doc["r1"] = relay1State;
    doc["r2"] = relay2State;
    doc["r3"] = relay3State;
    doc["r4"] = relay4State;

    doc["onCount"] = onCount;
    doc["offCount"] = offCount;
    doc["totalToggles"] = totalToggles;
    doc["gpio"] = 23;
    doc["ip"] = WiFi.localIP().toString();
    doc["ssid"] = WiFi.SSID();
    doc["uptime"] = millis() / 1000;
    String payload;
    serializeJson(doc, payload);

    int code = https.PUT(payload);
    Serial.printf("[FIREBASE] State synced! HTTP: %d | R1:%d R2:%d R3:%d R4:%d | Heap:%u\n",
      code, relay1State, relay2State, relay3State, relay4State, ESP.getFreeHeap());
    if (code == 200) {
      internetConnected = true;
    } else if (code <= 0) {
      internetConnected = false;
    }
    https.end();
  }
}

// Poll Firebase Realtime Database for remote control triggers
void pollFirebaseControl() {
  if (WiFi.status() != WL_CONNECTED || firebaseHost.length() < 8) return;

  WiFiClientSecure client;
  client.setInsecure();
  client.setHandshakeTimeout(5);
  HTTPClient https;

  String url = firebaseHost;
  if (!url.startsWith("http://") && !url.startsWith("https://")) {
    url = "https://" + url;
  }
  if (!url.endsWith("/")) url += "/";
  url += "ease_appliances/control.json";
  if (firebaseAuth.length() > 0) {
    url += "?auth=" + firebaseAuth;
  }

  if (https.begin(client, url)) {
    https.setTimeout(2500);
    int code = https.GET();
    if (code == 200) {
      internetConnected = true;
      String payload = https.getString();
      JsonDocument doc;
      DeserializationError err = deserializeJson(doc, payload);
      if (!err && doc.is<JsonObject>()) {
        // 0. Cloud unpair / factory reset trigger
        if ((doc["unpair"].is<bool>() && doc["unpair"].as<bool>()) ||
            (doc["reset"].is<bool>() && doc["reset"].as<bool>())) {
          https.end();
          unpairAndResetDevice();
          return;
        }

        bool changedAny = false;

        // 1. Direct r1, r2, r3, r4 keys
        if (doc["r1"].is<bool>()) {
          bool st = doc["r1"].as<bool>();
          if (st != relay1State) { relay1State = st; changedAny = true; }
        }
        if (doc["r2"].is<bool>()) {
          bool st = doc["r2"].as<bool>();
          if (st != relay2State) { relay2State = st; changedAny = true; }
        }
        if (doc["r3"].is<bool>()) {
          bool st = doc["r3"].as<bool>();
          if (st != relay3State) { relay3State = st; changedAny = true; }
        }
        if (doc["r4"].is<bool>()) {
          bool st = doc["r4"].as<bool>();
          if (st != relay4State) { relay4State = st; changedAny = true; }
        }

        // 2. Nested relays object: { "relays": { "r1": true, ... } }
        if (doc["relays"].is<JsonObject>()) {
          JsonObject ro = doc["relays"].as<JsonObject>();
          if (ro["r1"].is<bool>() && ro["r1"].as<bool>() != relay1State) { relay1State = ro["r1"].as<bool>(); changedAny = true; }
          if (ro["r2"].is<bool>() && ro["r2"].as<bool>() != relay2State) { relay2State = ro["r2"].as<bool>(); changedAny = true; }
          if (ro["r3"].is<bool>() && ro["r3"].as<bool>() != relay3State) { relay3State = ro["r3"].as<bool>(); changedAny = true; }
          if (ro["r4"].is<bool>() && ro["r4"].as<bool>() != relay4State) { relay4State = ro["r4"].as<bool>(); changedAny = true; }
        }

        // 3. Channel + state format: { "channel": 1, "state": true }
        if (doc["channel"].is<int>() && doc["state"].is<bool>()) {
          int ch = doc["channel"].as<int>();
          bool st = doc["state"].as<bool>();
          if (ch == 1 && relay1State != st) { relay1State = st; changedAny = true; }
          else if (ch == 2 && relay2State != st) { relay2State = st; changedAny = true; }
          else if (ch == 3 && relay3State != st) { relay3State = st; changedAny = true; }
          else if (ch == 4 && relay4State != st) { relay4State = st; changedAny = true; }
          else if (ch == 0) {
            relay1State = st; relay2State = st; relay3State = st; relay4State = st; changedAny = true;
          }
        }

        // 4. Legacy single-relay fallback ONLY if NO multi-channel keys exist at all
        bool hasChannelKeys = doc["r1"].is<bool>() || doc["r2"].is<bool>() ||
                              doc["r3"].is<bool>() || doc["r4"].is<bool>() ||
                              doc["relays"].is<JsonObject>() || doc["channel"].is<int>();
        if (!hasChannelKeys && doc["relay"].is<bool>()) {
          bool st = doc["relay"].as<bool>();
          if (relay1State != st || relay2State != st || relay3State != st || relay4State != st) {
            relay1State = st;
            relay2State = st;
            relay3State = st;
            relay4State = st;
            changedAny = true;
          }
        }

        if (changedAny) {
          applyRelayPins();
          prefs.putBool("r1", relay1State);
          prefs.putBool("r2", relay2State);
          prefs.putBool("r3", relay3State);
          prefs.putBool("r4", relay4State);
          bool anyOn = (relay1State || relay2State || relay3State || relay4State);
          if (anyOn) onCount++; else offCount++;
          totalToggles = onCount + offCount;
          prefs.putUInt("on_cnt", onCount);
          prefs.putUInt("off_cnt", offCount);
          prefs.putUInt("tot_tog", totalToggles);

          Serial.printf("[FIREBASE] Remote control applied: R1:%d R2:%d R3:%d R4:%d\n",
            relay1State, relay2State, relay3State, relay4State);

          if (pRelayChar) {
            char statusBuf[32];
            snprintf(statusBuf, sizeof(statusBuf), "R1:%d,R2:%d,R3:%d,R4:%d", relay1State, relay2State, relay3State, relay4State);
            pRelayChar->setValue(statusBuf);
            if (deviceConnected) pRelayChar->notify();
          }

          syncToFirebase();
        }
      }
    } else if (code <= 0) {
      internetConnected = false;
      Serial.printf("[FIREBASE] Poll failed! Code: %d | FreeHeap: %u\n", code, ESP.getFreeHeap());
    }
    https.end();
  }
}

// Set relay channel and apply pin state
void setRelayChannel(int ch, bool state, bool pushToCloud) {
  bool changed = false;
  if (ch == 1 && relay1State != state) { relay1State = state; changed = true; }
  else if (ch == 2 && relay2State != state) { relay2State = state; changed = true; }
  else if (ch == 3 && relay3State != state) { relay3State = state; changed = true; }
  else if (ch == 4 && relay4State != state) { relay4State = state; changed = true; }
  else if (ch == 0) { // All channels
    if (relay1State != state || relay2State != state || relay3State != state || relay4State != state) {
      relay1State = state;
      relay2State = state;
      relay3State = state;
      relay4State = state;
      changed = true;
    }
  }

  if (changed) {
    applyRelayPins();
    if (state) onCount++; else offCount++;
    totalToggles = onCount + offCount;
    prefs.putBool("r1", relay1State);
    prefs.putBool("r2", relay2State);
    prefs.putBool("r3", relay3State);
    prefs.putBool("r4", relay4State);
    prefs.putUInt("on_cnt", onCount);
    prefs.putUInt("off_cnt", offCount);
    prefs.putUInt("tot_tog", totalToggles);

    Serial.printf("[RELAY] Channel %d set to %s | R1:%d R2:%d R3:%d R4:%d\n",
      ch, state ? "ON" : "OFF", relay1State, relay2State, relay3State, relay4State);

    if (pRelayChar) {
      char statusBuf[32];
      snprintf(statusBuf, sizeof(statusBuf), "R1:%d,R2:%d,R3:%d,R4:%d", relay1State, relay2State, relay3State, relay4State);
      pRelayChar->setValue(statusBuf);
      if (deviceConnected) pRelayChar->notify();
    }

    if (pushToCloud && WiFi.status() == WL_CONNECTED && firebaseHost.length() > 5) {
      syncToFirebase();
    }
  }
}

// Unpair & Factory Reset: Disconnects Wi-Fi, clears NVS, and restarts into clean BLE provisioning mode
void unpairAndResetDevice() {
  Serial.println("\n[UNPAIR] =========================================");
  Serial.println("[UNPAIR] Unpair request received!");
  Serial.println("[UNPAIR] Erasing Wi-Fi settings, NVS, and restarting...");
  Serial.println("[UNPAIR] =========================================");

  // 1. Immediately turn OFF all 4 relays & test LED
  relay1State = false;
  relay2State = false;
  relay3State = false;
  relay4State = false;
  applyRelayPins();

  // 2. Notify Firebase Cloud that device is unpaired and offline
  if (WiFi.status() == WL_CONNECTED && firebaseHost.length() > 5) {
    WiFiClientSecure client;
    client.setInsecure();
    client.setHandshakeTimeout(3);
    HTTPClient https;
    String url = firebaseHost;
    if (!url.startsWith("http://") && !url.startsWith("https://")) url = "https://" + url;
    if (!url.endsWith("/")) url += "/";

    // Set state to unpaired and offline
    String stateUrl = url + "ease_appliances/state.json";
    if (firebaseAuth.length() > 0) stateUrl += "?auth=" + firebaseAuth;
    if (https.begin(client, stateUrl)) {
      https.setTimeout(2500);
      https.addHeader("Content-Type", "application/json");
      https.PUT("{\"online\":false,\"unpaired\":true,\"ip\":\"\",\"ssid\":\"\",\"last_seen\":0,\"r1\":false,\"r2\":false,\"r3\":false,\"r4\":false,\"relay\":false}");
      https.end();
    }

    // Reset control triggers
    String ctrlUrl = url + "ease_appliances/control.json";
    if (firebaseAuth.length() > 0) ctrlUrl += "?auth=" + firebaseAuth;
    if (https.begin(client, ctrlUrl)) {
      https.setTimeout(2500);
      https.addHeader("Content-Type", "application/json");
      https.PUT("{\"r1\":false,\"r2\":false,\"r3\":false,\"r4\":false,\"reset\":false,\"unpair\":false}");
      https.end();
    }
  }

  // 3. Clear Wi-Fi credentials & states from NVS Preferences
  prefs.remove("ssid");
  prefs.remove("pass");
  prefs.remove("r1");
  prefs.remove("r2");
  prefs.remove("r3");
  prefs.remove("r4");

  // 4. Erase ESP-IDF saved Wi-Fi and shutdown Wi-Fi radio
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  internetConnected = false;
  httpServerStarted = false;

  // 5. Update BLE status
  updateStatus("READY_FOR_PROVISIONING");
  if (pRelayChar) {
    pRelayChar->setValue("R1:0,R2:0,R3:0,R4:0");
  }

  // 6. Disconnect any active BLE client
  if (pServer && deviceConnected) {
    pServer->disconnect(0);
    deviceConnected = false;
  }

  Serial.println("[UNPAIR] Wiped successfully. Restarting into clean BLE provisioning mode...");
  delay(600);
  ESP.restart();
}

void setupHttpServer() {
  if (httpServerStarted) return;

  httpServer.on("/status", HTTP_GET, []() {
    JsonDocument doc;
    doc["name"] = DEVICE_NAME;
    doc["relay"] = (relay1State || relay2State || relay3State || relay4State);
    JsonObject relays = doc["relays"].to<JsonObject>();
    relays["r1"] = relay1State;
    relays["r2"] = relay2State;
    relays["r3"] = relay3State;
    relays["r4"] = relay4State;
    doc["r1"] = relay1State;
    doc["r2"] = relay2State;
    doc["r3"] = relay3State;
    doc["r4"] = relay4State;
    doc["gpio"] = 23;
    doc["onCount"] = onCount;
    doc["offCount"] = offCount;
    doc["totalToggles"] = totalToggles;
    doc["ip"] = WiFi.localIP().toString();
    doc["ssid"] = WiFi.SSID();
    doc["rssi"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    doc["firebaseConfigured"] = (firebaseHost.length() > 0);
    doc["firebaseHost"] = firebaseHost;
    doc["freeHeap"] = ESP.getFreeHeap();
    String res;
    serializeJson(doc, res);
    httpServer.sendHeader("Access-Control-Allow-Origin", "*");
    httpServer.send(200, "application/json", res);
  });

  httpServer.on("/relay", HTTP_GET, []() {
    int ch = 0;
    if (httpServer.hasArg("ch")) ch = httpServer.arg("ch").toInt();
    else if (httpServer.hasArg("channel")) ch = httpServer.arg("channel").toInt();

    if (httpServer.hasArg("state")) {
      String s = httpServer.arg("state");
      s.toLowerCase();
      bool target = (s == "on" || s == "1" || s == "true");
      setRelayChannel(ch, target, true);
    } else if (httpServer.hasArg("toggle")) {
      if (ch == 1) setRelayChannel(1, !relay1State, true);
      else if (ch == 2) setRelayChannel(2, !relay2State, true);
      else if (ch == 3) setRelayChannel(3, !relay3State, true);
      else if (ch == 4) setRelayChannel(4, !relay4State, true);
      else setRelayChannel(0, !(relay1State || relay2State || relay3State || relay4State), true);
    }

    JsonDocument doc;
    doc["relay"] = (relay1State || relay2State || relay3State || relay4State);
    JsonObject relays = doc["relays"].to<JsonObject>();
    relays["r1"] = relay1State;
    relays["r2"] = relay2State;
    relays["r3"] = relay3State;
    relays["r4"] = relay4State;
    doc["r1"] = relay1State;
    doc["r2"] = relay2State;
    doc["r3"] = relay3State;
    doc["r4"] = relay4State;
    doc["onCount"] = onCount;
    doc["offCount"] = offCount;
    doc["totalToggles"] = totalToggles;
    String res;
    serializeJson(doc, res);
    httpServer.sendHeader("Access-Control-Allow-Origin", "*");
    httpServer.send(200, "application/json", res);
  });

  httpServer.on("/config/firebase", HTTP_GET, []() {
    if (httpServer.hasArg("host")) {
      firebaseHost = httpServer.arg("host");
      prefs.putString("fb_host", firebaseHost);
    }
    if (httpServer.hasArg("auth")) {
      firebaseAuth = httpServer.arg("auth");
      prefs.putString("fb_auth", firebaseAuth);
    }
    if (httpServer.hasArg("clear")) {
      firebaseHost = "";
      firebaseAuth = "";
      prefs.remove("fb_host");
      prefs.remove("fb_auth");
    }

    if (firebaseHost.length() > 0) {
      syncToFirebase();
    }

    JsonDocument doc;
    doc["firebaseConfigured"] = (firebaseHost.length() > 0);
    doc["firebaseHost"] = firebaseHost;
    String res;
    serializeJson(doc, res);
    httpServer.sendHeader("Access-Control-Allow-Origin", "*");
    httpServer.send(200, "application/json", res);
  });

  httpServer.on("/stats/reset", HTTP_GET, []() {
    onCount = 0;
    offCount = 0;
    totalToggles = 0;
    prefs.putUInt("on_cnt", 0);
    prefs.putUInt("off_cnt", 0);
    prefs.putUInt("tot_tog", 0);

    syncToFirebase();

    JsonDocument doc;
    doc["onCount"] = onCount;
    doc["offCount"] = offCount;
    doc["totalToggles"] = totalToggles;
    String res;
    serializeJson(doc, res);
    httpServer.sendHeader("Access-Control-Allow-Origin", "*");
    httpServer.send(200, "application/json", res);
  });

  httpServer.on("/unpair", HTTP_GET, []() {
    httpServer.sendHeader("Access-Control-Allow-Origin", "*");
    httpServer.send(200, "application/json", "{\"success\":true,\"message\":\"Unpairing and wiping device\"}");
    delay(200);
    unpairAndResetDevice();
  });

  httpServer.on("/reset", HTTP_GET, []() {
    httpServer.sendHeader("Access-Control-Allow-Origin", "*");
    httpServer.send(200, "application/json", "{\"success\":true,\"message\":\"Unpairing and wiping device\"}");
    delay(200);
    unpairAndResetDevice();
  });

  httpServer.onNotFound([]() {
    httpServer.sendHeader("Access-Control-Allow-Origin", "*");
    httpServer.send(404, "text/plain", "Not Found");
  });

  httpServer.begin();
  httpServerStarted = true;
  Serial.println("[HTTP] Local REST Server started on port 80");
}

// BLE Server Callbacks
class MyServerCallbacks: public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer) override {
    deviceConnected = true;
    Serial.println("[BLE] Phone / Web Connected!");
    updateStatus(getFullStatus());
  }

  void onDisconnect(NimBLEServer* pServer) override {
    deviceConnected = false;
    Serial.println("[BLE] Client Disconnected.");
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[BLE] Resuming Advertising for setup...");
      NimBLEDevice::startAdvertising();
    } else {
      Serial.println("[BLE] Wi-Fi is connected - BLE stays hidden for security.");
    }
  }
};

// Wi-Fi Provisioning Characteristic Callback
class WifiProvCallbacks: public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) override {
    std::string val = pCharacteristic->getValue();
    if (val.length() > 0) {
      String value = String(val.c_str());
      value.trim();
      Serial.print("[BLE PROV] Received: ");
      Serial.println(value);

      if (value.equalsIgnoreCase("UNPAIR") || value.equalsIgnoreCase("RESET")) {
        unpairAndResetDevice();
        return;
      }

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, value);
      if (!error) {
        if ((doc["unpair"].is<bool>() && doc["unpair"].as<bool>()) ||
            (doc["reset"].is<bool>() && doc["reset"].as<bool>()) ||
            (doc["action"].is<const char*>() && strcmp(doc["action"].as<const char*>(), "unpair") == 0)) {
          unpairAndResetDevice();
          return;
        }
        if (doc["ssid"].is<const char*>()) {
          newSsid = doc["ssid"].as<String>();
          newPass = doc["password"].is<const char*>() ? doc["password"].as<String>() : "";
          if (doc["firebaseHost"].is<const char*>()) {
            firebaseHost = doc["firebaseHost"].as<String>();
            prefs.putString("fb_host", firebaseHost);
          }
          pendingWifiConnect = true;
          return;
        }
      }

      int separatorIdx = value.indexOf(':');
      if (separatorIdx != -1) {
        newSsid = value.substring(0, separatorIdx);
        newPass = value.substring(separatorIdx + 1);
        pendingWifiConnect = true;
      } else {
        newSsid = value;
        newPass = "";
        pendingWifiConnect = true;
      }
    }
  }
};

// Relay Control Callback
class RelayCallbacks: public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) override {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[BLE] Power control rejected: ESP32 has no Wi-Fi / Internet connection");
      return;
    }
    std::string val = pCharacteristic->getValue();
    String value = String(val.c_str());
    value.toUpperCase();
    if (value.startsWith("R1:")) setRelayChannel(1, value.substring(3) == "1" || value.substring(3) == "ON");
    else if (value.startsWith("R2:")) setRelayChannel(2, value.substring(3) == "1" || value.substring(3) == "ON");
    else if (value.startsWith("R3:")) setRelayChannel(3, value.substring(3) == "1" || value.substring(3) == "ON");
    else if (value.startsWith("R4:")) setRelayChannel(4, value.substring(3) == "1" || value.substring(3) == "ON");
    else if (value == "1" || value == "ON" || value == "TRUE") setRelayChannel(0, true);
    else if (value == "0" || value == "OFF" || value == "FALSE") setRelayChannel(0, false);
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=================================");
  Serial.println("  Ease Appliances - 4 Channel Relay");
  Serial.println("  Lightweight NimBLE + Cloud Engine");
  Serial.println("=================================");

  // Initialize NVS storage
  prefs.begin("ease_app", false);
  String savedSsid = prefs.getString("ssid", "");
  String savedPass = prefs.getString("pass", "");
  firebaseHost = prefs.getString("fb_host", DEFAULT_FIREBASE_HOST);
  if (firebaseHost.length() < 8) {
    firebaseHost = DEFAULT_FIREBASE_HOST;
  }
  firebaseAuth = prefs.getString("fb_auth", "");
  onCount = prefs.getUInt("on_cnt", 0);
  offCount = prefs.getUInt("off_cnt", 0);
  totalToggles = prefs.getUInt("tot_tog", onCount + offCount);

  // Restore saved relay states (default false / OFF)
  relay1State = prefs.getBool("r1", false);
  relay2State = prefs.getBool("r2", false);
  relay3State = prefs.getBool("r3", false);
  relay4State = prefs.getBool("r4", false);

  // Initialize 4 Relay GPIO Pins & Onboard Test LED
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  pinMode(TEST_LED_PIN, OUTPUT);

  // Apply pin states (Active LOW: LOW = ON, HIGH = OFF)
  applyRelayPins();

  // Initialize NimBLE
  NimBLEDevice::init(DEVICE_NAME);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService *pService = pServer->createService(SERVICE_UUID);

  NimBLECharacteristic *pProvChar = pService->createCharacteristic(
    CHAR_WIFI_PROV_UUID,
    NIMBLE_PROPERTY::WRITE
  );
  pProvChar->setCallbacks(new WifiProvCallbacks());

  pStatusChar = pService->createCharacteristic(
    CHAR_STATUS_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  pStatusChar->setValue(getFullStatus().c_str());

  pRelayChar = pService->createCharacteristic(
    CHAR_RELAY_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
  );
  pRelayChar->setCallbacks(new RelayCallbacks());
  pRelayChar->setValue("R1:0,R2:0,R3:0,R4:0");

  pService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();

  Serial.printf("[BLE] NimBLE Started! Free Heap: %u bytes\n", ESP.getFreeHeap());

  // Try auto-connecting to saved Wi-Fi if available
  if (savedSsid.length() > 0) {
    Serial.printf("[WIFI] Auto-connecting to saved network: %s\n", savedSsid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(savedSsid.c_str(), savedPass.c_str());
  } else {
    Serial.println("[WIFI] No saved credentials. Awaiting Bluetooth provisioning...");
  }
}

void loop() {
  // 1. Handle Wi-Fi provisioning requests received over BLE
  if (pendingWifiConnect) {
    pendingWifiConnect = false;
    Serial.printf("[PROV] Connecting to SSID: '%s'...\n", newSsid.c_str());
    updateStatus("CONNECTING");

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(newSsid.c_str(), newPass.c_str());

    unsigned long startMs = millis();
    bool connected = false;
    while (millis() - startMs < 12000) {
      if (WiFi.status() == WL_CONNECTED) {
        connected = true;
        break;
      }
      delay(300);
      Serial.print(".");
    }
    Serial.println();

    if (connected) {
      Serial.printf("[WIFI] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
      prefs.putString("ssid", newSsid);
      prefs.putString("pass", newPass);

      setupHttpServer();

      // Test internet connection via Firebase heartbeat push
      syncToFirebase();
      updateStatus(getFullStatus());
    } else {
      Serial.println("[WIFI] Connection Failed!");
      updateStatus("CONNECT_FAILED");
    }
  }

  // 2. Start local HTTP server once Wi-Fi connects
  if (WiFi.status() == WL_CONNECTED && !httpServerStarted) {
    setupHttpServer();
  }

  // 3. Handle local HTTP client requests
  if (httpServerStarted) {
    httpServer.handleClient();
  }

  // 4. Cloud Synchronizer & Remote Control Polling (when connected to Wi-Fi)
  if (WiFi.status() == WL_CONNECTED && firebaseHost.length() > 0) {
    static unsigned long lastHeartbeatPush = 0;
    static unsigned long lastControlPoll = 0;
    unsigned long now = millis();

    // Heartbeat every 6 seconds, control poll every 2 seconds in between
    if (now - lastHeartbeatPush >= 6000) {
      lastHeartbeatPush = now;
      syncToFirebase();
    } else if (now - lastControlPoll >= 2000) {
      lastControlPoll = now;
      pollFirebaseControl();
    }
  }

  // 5. Watchdog for Wi-Fi reconnection if router drops
  static unsigned long lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 15000) {
    lastWifiCheck = millis();
    String saved = prefs.getString("ssid", "");
    if (WiFi.status() != WL_CONNECTED && saved.length() > 0) {
      Serial.println("[WIFI] Disconnected from AP. Reconnecting...");
      WiFi.reconnect();
    }
  }

  // 6. Security Watchdog: Auto-Shutoff BLE Advertising once Wi-Fi connects
  if (WiFi.status() == WL_CONNECTED) {
    if (NimBLEDevice::getAdvertising()->isAdvertising()) {
      Serial.println("[SECURITY] Wi-Fi is connected! Stopping BLE advertising for safety.");
      NimBLEDevice::getAdvertising()->stop();
    }
  } else {
    // If Wi-Fi is lost/disconnected and no device is connected, resume BLE so owner can configure
    if (!NimBLEDevice::getAdvertising()->isAdvertising() && !deviceConnected) {
      Serial.println("[SECURITY] Wi-Fi offline. Resuming BLE advertising for setup...");
      NimBLEDevice::getAdvertising()->start();
    }
  }

  delay(20);
}
