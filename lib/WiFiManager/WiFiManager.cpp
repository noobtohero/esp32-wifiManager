#include "WiFiManager.h"
#include <Preferences.h>

WiFiManager wifiManager;

WiFiManager::WiFiManager()
    : _server(80), _events("/events"), _portalRunning(false),
      _shouldRestart(false), _taskHandle(nullptr) {}

WiFiManager::~WiFiManager() {
  if (_taskHandle)
    vTaskDelete(_taskHandle);
}

bool WiFiManager::begin(const char *apName, SimpleCallback onConnect) {
  _connectedCb = onConnect;
  return begin(apName, nullptr);
}

bool WiFiManager::begin(const char *apName, const char *apPassword) {
  _apName = apName;
  _apPassword = apPassword ? apPassword : "";

  WM_LOG("\n[WiFiManager] Starting...");

  // 1. Initialize File System
  if (!LittleFS.begin(true)) {
    WM_LOG("[WiFiManager] ERROR: LittleFS Mount Failed");
  }

  // 2. Try Auto-connecting to Last 3 Networks
  Preferences prefs;
  // เปลี่ยนเป็น false เพื่อให้สร้าง namespace ใหม่ได้ถ้ายังไม่มี (ช่วยแก้ปัญหา nvs_open failed:
  // NOT_FOUND)
  prefs.begin("wifi-manager", false);
  String networksJson = prefs.getString("networks", "[]");
  prefs.end();

  JsonDocument doc;
  deserializeJson(doc, networksJson);
  JsonArray networks = doc.as<JsonArray>();

  if (networks.size() > 0) {
    WiFi.mode(WIFI_STA);
    for (JsonObject net : networks) {
      String ssid = net["ssid"];
      String pass = net["pass"];

      WM_LOGF("[WiFiManager] Trying network: %s\n", ssid.c_str());
      WiFi.begin(ssid.c_str(), pass.c_str());

      unsigned long startAttemptTime = millis();
      bool connected = false;
      while (millis() - startAttemptTime < 10000) { // 10s timeout per network
        if (WiFi.status() == WL_CONNECTED) {
          connected = true;
          break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        WM_LOGF(".");
      }

      if (WiFi.status() == WL_CONNECTED) {
        WM_LOG("\n[WiFiManager] Connected successfully!");
        WiFiManager::setSleep(true);
        if (_statusCallback)
          _statusCallback(CONNECTED);
        if (_connectedCb)
          _connectedCb(); // Specialized Callback
        if (_callback)
          _callback(true);
        return true;
      }
      WM_LOG("\n[WiFiManager] Failed to connect to " + ssid);
    }
  }

  // 4. Start Portal if all connections failed
  startAP();
  startPortal();

  if (_statusCallback)
    _statusCallback(PORTAL_START);
  if (_portalCb)
    _portalCb(); // Specialized Callback

  // Start FreeRTOS task for background management (Scan + DNS)
  xTaskCreate(wifiTask, "wifi_task", 4096, this, 1, &_taskHandle);

  if (_callback)
    _callback(false);
  return false;
}

void WiFiManager::resetSettings() {
  Preferences prefs;
  prefs.begin("wifi-manager", false);
  prefs.clear();
  prefs.end();
  WM_LOG("[WiFiManager] Settings reset. Restarting...");
  delay(1000);
  ESP.restart();
}

void WiFiManager::startAP() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(_apName.c_str(),
              _apPassword.length() > 0 ? _apPassword.c_str() : nullptr);

  // Captive Portal IP
  IPAddress IP = WiFi.softAPIP();
  WM_LOGF("[WiFiManager] AP Active: %s\n", _apName.c_str());
  WM_LOGF("[WiFiManager] Portal IP: ");
  WM_LOG(IP);

  _dnsServer.start(DNS_PORT, "*", IP);
}

void WiFiManager::stopPortal() {
  if (_portalRunning) {
    _dnsServer.stop();
    _server.end();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    _portalRunning = false;
    if (_statusCallback)
      _statusCallback(PORTAL_TIMEOUT);
    if (_timeoutCb)
      _timeoutCb(); // Specialized Callback
    if (_connectedCb)
      _connectedCb(); // Trigger connected if stopped after configuration
    WM_LOG("[WiFiManager] Portal & AP Stopped. Entering Low Power STA mode.");
    WiFi.setSleep(true); // Ensure modem sleep is on
  }
}

void WiFiManager::startPortal() {
  setupRoutes();
  _server.addHandler(&_events);
  _server.begin();
  _portalRunning = true;
}

WiFiManager &WiFiManager::setStatusLED(int pin, bool activeLow) {
  _ledPin = pin;
  _ledInvert = activeLow;
  pinMode(_ledPin, OUTPUT);
  // Initial state (OFF)
  digitalWrite(_ledPin, _ledInvert ? HIGH : LOW);
  return *this;
}

void WiFiManager::setSleep(bool enable) {
  _sleepEnabled = enable;
  WiFi.setSleep(enable);
  if (_sleepCallback)
    _sleepCallback(enable);
  WM_LOGF("[WiFiManager] Modem Sleep is now: %s\n",
          enable ? "ENABLED" : "DISABLED");
}

bool WiFiManager::isConnected() { return WiFi.status() == WL_CONNECTED; }

String WiFiManager::getSSID() { return WiFi.SSID(); }

void WiFiManager::setupRoutes() {
  // Serve static files from LittleFS
  _server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  // Save configuration
  _server.on("/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true)) {
      String s = request->getParam("ssid", true)->value();
      String p = request->hasParam("password", true)
                     ? request->getParam("password", true)->value()
                     : "";

      // Update networks list (last 3)
      Preferences prefs;
      prefs.begin("wifi-manager", false);
      String networksJson = prefs.getString("networks", "[]");

      JsonDocument doc;
      deserializeJson(doc, networksJson);
      JsonArray networks = doc.as<JsonArray>();

      // Remove if SSID already exists to move it to the top
      for (size_t i = 0; i < networks.size(); i++) {
        if (networks[i]["ssid"] == s) {
          networks.remove(i);
          break;
        }
      }

      // Create new entry
      JsonDocument newEntryDoc;
      JsonObject newEntry = newEntryDoc.to<JsonObject>();
      newEntry["ssid"] = s;
      newEntry["pass"] = p;

      // Insert at the beginning
      JsonDocument finalDoc;
      JsonArray finalArray = finalDoc.to<JsonArray>();
      finalArray.add(newEntry);
      for (JsonObject v : networks) {
        if (finalArray.size() < 3) {
          finalArray.add(v);
        }
      }

      String output;
      serializeJson(finalDoc, output);
      prefs.putString("networks", output);
      prefs.end();

      request->send(200, "text/plain", "OK");

      // ค่อยส่ง flag ให้ restart ใน loop ของ task
      _shouldRestart = true;
    } else {
      request->send(400, "text/plain", "Mising SSID");
    }
  });

  // Captive Portal Redirect
  _server.onNotFound(
      [](AsyncWebServerRequest *request) { request->redirect("/"); });
}

void WiFiManager::emitWiFiFound(int i) {
  JsonDocument doc;
  doc["ssid"] = WiFi.SSID(i);
  doc["rssi"] = WiFi.RSSI(i);
  doc["secure"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;

  String json;
  serializeJson(doc, json);
  _events.send(json.c_str(), "wifi-found", millis());
}

void WiFiManager::wifiTask(void *pvParameters) {
  WiFiManager *instance = (WiFiManager *)pvParameters;

  TickType_t lastScanTime = 0;
  unsigned long apStartTime = millis();
  const unsigned long AP_TIMEOUT = 300000; // 5 minutes timeout for Power Saving

  while (true) {
    // Safe Restart Check
    if (instance->_shouldRestart) {
      vTaskDelay(pdMS_TO_TICKS(2000));
      ESP.restart();
    }

    // Auto-AP Timeout (Low Energy Feature)
    // If portal is running but no one is connected/configuring for X minutes,
    // we can stop AP to save power if device is battery powered.
    if (instance->_portalRunning && (millis() - apStartTime > AP_TIMEOUT)) {
      if (WiFi.softAPgetStationNum() == 0) {
        WM_LOG("[WiFiManager] AP Timeout - No stations connected. Shutting "
               "down AP for energy saving.");
        instance->stopPortal();
      } else {
        apStartTime = millis(); // Reset timer if someone is connected
      }
    }

    // 1. Process DNS for Captive Portal
    if (instance->_portalRunning) {
      instance->_dnsServer.processNextRequest();
    }

    // 2. Smooth Background Scanning
    if (instance->_portalRunning &&
        (millis() - lastScanTime > 15000 || lastScanTime == 0)) {
      lastScanTime = millis();
      WM_LOG("[WiFiManager] Scanning nearby networks...");

      // Non-blocking scan start
      int n = WiFi.scanNetworks(false, false, false, 200);

      for (int i = 0; i < n; ++i) {
        instance->emitWiFiFound(i);
        vTaskDelay(pdMS_TO_TICKS(150)); // Visual "Smooth" spacing
      }
      WiFi.scanDelete();
    }

    // 3. Monitor WiFi Status & LED Management
    static bool lastConnected = false;
    static unsigned long lastBlink = 0;
    static bool ledState = false;

    bool currentlyConnected = (WiFi.status() == WL_CONNECTED);

    // Trigger Callbacks
    if (!instance->_portalRunning && currentlyConnected != lastConnected) {
      lastConnected = currentlyConnected;
      if (currentlyConnected) {
        if (instance->_statusCallback)
          instance->_statusCallback(CONNECTED);
        if (instance->_connectedCb)
          instance->_connectedCb();
      } else {
        if (instance->_statusCallback)
          instance->_statusCallback(DISCONNECTED);
        if (instance->_disconnectedCb)
          instance->_disconnectedCb();
      }
    }

    // LED Pattern Management
    if (instance->_ledPin != -1) {
      if (currentlyConnected) {
        digitalWrite(instance->_ledPin,
                     instance->_ledInvert ? LOW : HIGH); // Solid ON
      } else {
        // Blink Pattern
        int interval = instance->_portalRunning
                           ? 150
                           : 800; // Fast for Portal, Slow for Connecting
        if (millis() - lastBlink > interval) {
          lastBlink = millis();
          ledState = !ledState;
          digitalWrite(instance->_ledPin,
                       instance->_ledInvert ? !ledState : ledState);
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
