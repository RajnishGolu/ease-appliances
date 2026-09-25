#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

#define DEVICE_NAME "Ease Appliances"

// GPIO Pins - Using Onboard Blue LED (GPIO 2) as the single test relay pin!
#define TEST_RELAY_PIN 2

// UUIDs
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_WIFI_PROV_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_STATUS_UUID       "beb5483f-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_RELAY_UUID        "beb54840-36e1-4688-b7f5-ea07361b26a8"

Preferences prefs;
BLEServer* pServer = nullptr;
BLECharacteristic* pStatusChar = nullptr;
BLECharacteristic* pRelayChar = nullptr;
WebServer httpServer(80);

bool deviceConnected = false;
bool oldDeviceConnected = false;
bool relayState = true;
bool httpServerStarted = false;

// Persistent Counters & Tracking
uint32_t onCount = 0;
uint32_t offCount = 0;
uint32_t totalToggles = 0;

// Firebase Cloud Sync
const char* DEFAULT_FIREBASE_HOST = "https://smart-plug-c131f-default-rtdb.firebaseio.com";
String firebaseHost = DEFAULT_FIREBASE_HOST;
String firebaseAuth = ""; // optional auth secret
unsigned long lastFirebasePoll = 0;
unsigned long lastTlsTime = 0;
bool internetConnected = false;

String newSsid = "";
String newPass = "";
bool pendingWifiConnect = false;

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
    doc["relay"] = relayState;
    doc["onCount"] = onCount;
    doc["offCount"] = offCount;
    doc["totalToggles"] = totalToggles;
    doc["gpio"] = TEST_RELAY_PIN;
    doc["ip"] = WiFi.localIP().toString();
    doc["ssid"] = WiFi.SSID();
    doc["uptime"] = millis() / 1000;
    String payload;
    serializeJson(doc, payload);

    int code = https.PUT(payload);
    Serial.printf("[FIREBASE] State synced! HTTP result: %d\n", code);
    if (code == 200) {
      internetConnected = true;
    } else if (code <= 0) {
      internetConnected = false;
    }
    https.end();
    lastTlsTime = millis();
  }
}

// Poll Firebase Realtime Database for remote control triggers
void pollFirebaseControl() {
  if (WiFi.status() != WL_CONNECTED || firebaseHost.length() < 8) return;
  if (millis() - lastFirebasePoll < 2000) return;
  if (millis() - lastTlsTime < 1200) return; // Prevent overlapping TLS handshakes
  lastFirebasePoll = millis();

  WiFiClientSecure client;
  client.setInsecure();
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

  bool shouldTriggerRelay = false;
  bool targetRelayState = false;

  if (https.begin(client, url)) {
    https.setTimeout(2500);
    int code = https.GET();
    if (code == 200) {
      internetConnected = true;
      String payload = https.getString();
      JsonDocument doc;
      DeserializationError err = deserializeJson(doc, payload);
      if (!err) {
        if (doc.is<JsonObject>() && doc["relay"].is<bool>()) {
          bool remoteRelay = doc["relay"].as<bool>();
          if (remoteRelay != relayState) {
            shouldTriggerRelay = true;
            targetRelayState = remoteRelay;
          }
        } else if (doc.is<bool>()) {
          bool remoteRelay = doc.as<bool>();
          if (remoteRelay != relayState) {
            shouldTriggerRelay = true;
            targetRelayState = remoteRelay;
          }
        }
      }
    } else if (code <= 0) {
      internetConnected = false;
    }
    https.end(); // Safe: End GET request before setRelay triggers syncToFirebase
    lastTlsTime = millis();
  }

  if (shouldTriggerRelay) {
    Serial.printf("[FIREBASE] Remote control trigger: %s\n", targetRelayState ? "ON" : "OFF");
    void setRelay(bool state, bool pushToCloud);
    setRelay(targetRelayState, true);
  }
}

// Set relay and LED state, maintaining persistent on/off counters
void setRelay(bool state, bool pushToCloud = true) {
  bool changed = (relayState != state);
  relayState = state;
  digitalWrite(TEST_RELAY_PIN, relayState ? HIGH : LOW);

  if (changed) {
    if (relayState) {
      onCount++;
      prefs.putUInt("on_cnt", onCount);
    } else {
      offCount++;
      prefs.putUInt("off_cnt", offCount);
    }
    totalToggles = onCount + offCount;
    prefs.putUInt("tot_tog", totalToggles);
  }

  Serial.printf("[RELAY] State: %s (GPIO %d) | ON: %u, OFF: %u, Total: %u\n",
    relayState ? "ON" : "OFF", TEST_RELAY_PIN, onCount, offCount, totalToggles);

  if (pRelayChar) {
    pRelayChar->setValue(relayState ? "ON" : "OFF");
    if (deviceConnected) {
      pRelayChar->notify();
    }
  }

  if (pushToCloud && WiFi.status() == WL_CONNECTED && firebaseHost.length() > 5) {
    syncToFirebase();
  }
}

void setupHttpServer() {
  if (httpServerStarted) return;

  httpServer.on("/status", HTTP_GET, []() {
    JsonDocument doc;
    doc["name"] = DEVICE_NAME;
    doc["relay"] = relayState;
    doc["ip"] = WiFi.localIP().toString();
    doc["ssid"] = WiFi.SSID();
    doc["gpio"] = TEST_RELAY_PIN;
    doc["onCount"] = onCount;
    doc["offCount"] = offCount;
    doc["totalToggles"] = totalToggles;
    doc["firebaseConfigured"] = (firebaseHost.length() > 0);
    doc["firebaseHost"] = firebaseHost;
    doc["wifiConnected"] = (WiFi.status() == WL_CONNECTED);
    String res;
    serializeJson(doc, res);
    httpServer.sendHeader("Access-Control-Allow-Origin", "*");
    httpServer.send(200, "application/json", res);
  });

  httpServer.on("/relay", HTTP_GET, []() {
    if (httpServer.hasArg("state")) {
      String st = httpServer.arg("state");
      st.toUpperCase();
      setRelay(st == "1" || st == "ON" || st == "TRUE", true);
    } else if (httpServer.hasArg("toggle")) {
      setRelay(!relayState, true);
    }
    JsonDocument doc;
    doc["name"] = DEVICE_NAME;
    doc["relay"] = relayState;
    doc["gpio"] = TEST_RELAY_PIN;
    doc["onCount"] = onCount;
    doc["offCount"] = offCount;
    doc["totalToggles"] = totalToggles;
    doc["ip"] = WiFi.localIP().toString();
    String res;
    serializeJson(doc, res);
    httpServer.sendHeader("Access-Control-Allow-Origin", "*");
    httpServer.send(200, "application/json", res);
  });

  // Endpoint to save Firebase RTDB configuration
  httpServer.on("/firebase/config", HTTP_GET, []() {
    if (httpServer.hasArg("host")) {
      firebaseHost = httpServer.arg("host");
      if (!firebaseHost.startsWith("http://") && !firebaseHost.startsWith("https://")) {
        firebaseHost = "https://" + firebaseHost;
      }
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

  // Endpoint to reset on/off counters
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

  httpServer.onNotFound([]() {
    httpServer.sendHeader("Access-Control-Allow-Origin", "*");
    httpServer.send(404, "text/plain", "Not Found");
  });

  httpServer.begin();
  httpServerStarted = true;
  Serial.println("[HTTP] Local REST Server started on port 80");
}

// BLE Server Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true;
    Serial.println("[BLE] Client Connected!");
    updateStatus(getFullStatus());
  }

  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    Serial.println("[BLE] Client Disconnected. Restarting advertising...");
    delay(100);
    pServer->startAdvertising();
  }
};

// Wi-Fi Provisioning Callback
class WifiProvCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    String value = pCharacteristic->getValue().c_str();
    if (value.length() > 0) {
      Serial.print("[BLE] Received Wi-Fi payload: ");
      Serial.println(value);

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, value);
      if (!error && doc["ssid"].is<const char*>()) {
        newSsid = doc["ssid"].as<String>();
        newPass = doc["password"].is<const char*>() ? doc["password"].as<String>() : "";
        if (doc["firebaseHost"].is<const char*>()) {
          firebaseHost = doc["firebaseHost"].as<String>();
          prefs.putString("fb_host", firebaseHost);
        }
        pendingWifiConnect = true;
      } else {
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
  }
};

// Relay Control Callback
class RelayCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    // Bluetooth is ONLY for Wi-Fi provisioning. Power control requires Wi-Fi / Internet!
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[BLE] Power control rejected: ESP32 has no Wi-Fi / Internet connection");
      return;
    }
    String value = pCharacteristic->getValue().c_str();
    value.toUpperCase();
    if (value == "1" || value == "ON" || value == "TRUE") {
      setRelay(true, true);
    } else if (value == "0" || value == "OFF" || value == "FALSE") {
      setRelay(false, true);
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=================================");
  Serial.println("  Ease Appliances - Smart Plug");
  Serial.println("  Testing on GPIO 2 (Blue LED)");
  Serial.println("=================================");

  // Initialize GPIO 2 (Onboard Blue LED used for test)
  pinMode(TEST_RELAY_PIN, OUTPUT);

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

  // Set initial state
  setRelay(true, false);

  // Initialize BLE
  BLEDevice::init(DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Wi-Fi Provisioning Characteristic (Write)
  BLECharacteristic *pProvChar = pService->createCharacteristic(
    CHAR_WIFI_PROV_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pProvChar->setCallbacks(new WifiProvCallbacks());

  // Status Characteristic (Read, Notify)
  pStatusChar = pService->createCharacteristic(
    CHAR_STATUS_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pStatusChar->addDescriptor(new BLE2902());
  pStatusChar->setValue(getFullStatus().c_str());

  // Relay Control Characteristic (Read, Write, Notify)
  pRelayChar = pService->createCharacteristic(
    CHAR_RELAY_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );
  pRelayChar->addDescriptor(new BLE2902());
  pRelayChar->setCallbacks(new RelayCallbacks());
  pRelayChar->setValue("ON");

  // Start BLE Service
  pService->start();

  // Start Advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Advertising started as: " DEVICE_NAME);

  // Try auto-connecting to saved Wi-Fi if available
  if (savedSsid.length() > 0) {
    Serial.printf("[WIFI] Auto-connecting to saved network: %s\n", savedSsid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(savedSsid.c_str(), savedPass.c_str());

    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
      delay(500);
      Serial.print(".");
      timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\n[WIFI] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
      setupHttpServer();
      updateStatus(getFullStatus());
      syncToFirebase();
    } else {
      Serial.println("\n[WIFI] Auto-connect timed out. Awaiting BLE provisioning.");
      updateStatus("READY_FOR_PROVISIONING");
    }
  } else {
    Serial.println("[WIFI] No saved credentials found. Ready for BLE provisioning.");
    updateStatus("READY_FOR_PROVISIONING");
  }
}

void loop() {
  // Disconnect/Reconnect handling for BLE advertising
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("[BLE] Restarted advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  // Handle incoming HTTP client requests
  if (httpServerStarted && WiFi.status() == WL_CONNECTED) {
    httpServer.handleClient();
  }

  // Poll Firebase Realtime Database
  if (WiFi.status() == WL_CONNECTED) {
    pollFirebaseControl();

    // Periodic heartbeat to Firebase every 10 seconds
    static unsigned long lastHeartbeatPush = 0;
    if (firebaseHost.length() > 5 && (millis() - lastHeartbeatPush > 10000)) {
      if (millis() - lastTlsTime >= 1500) {
        lastHeartbeatPush = millis();
        syncToFirebase();
      }
    }
  }

  // Handle incoming Wi-Fi configuration request
  if (pendingWifiConnect) {
    pendingWifiConnect = false;
    Serial.printf("[WIFI] Provisioning to SSID: '%s'\n", newSsid.c_str());
    updateStatus("CONNECTING");

    WiFi.disconnect(true);
    delay(200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(newSsid.c_str(), newPass.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
      delay(500);
      Serial.print(".");
      digitalWrite(TEST_RELAY_PIN, !digitalRead(TEST_RELAY_PIN));
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      String ip = WiFi.localIP().toString();
      Serial.printf("\n[WIFI] Success! Assigned IP: %s\n", ip.c_str());

      prefs.putString("ssid", newSsid);
      prefs.putString("pass", newPass);

      digitalWrite(TEST_RELAY_PIN, HIGH);
      setupHttpServer();
      updateStatus(getFullStatus());
      syncToFirebase();
    } else {
      Serial.println("\n[WIFI] Connection Failed!");
      digitalWrite(TEST_RELAY_PIN, LOW);
      updateStatus("CONNECT_FAILED");
    }

    // Ensure BLE advertising is active after Wi-Fi provisioning
    if (pServer && !deviceConnected) {
      pServer->startAdvertising();
    }
  }

  delay(10);
}
