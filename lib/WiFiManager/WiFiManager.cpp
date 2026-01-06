#include "WiFiManager.h"
#include "WebAssets.h"
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>
#include <functional>
#include <time.h>


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
  _isConnecting = true;

  WM_LOG("\n[WiFiManager] Starting...");

  // Initialize Time Sync settings
  initTime();

  // Start FreeRTOS task early for background management (LED + Scan + DNS +
  // Time)
  if (!_taskHandle) {
    xTaskCreate(wifiTask, "wifi_task", 4096, this, 1, &_taskHandle);
  }

  // 1. Initialize System (LittleFS removed for plug-and-play)

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
        _isConnecting = false;
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
  _isConnecting = false;
  startAP();
  startPortal();

  if (_statusCallback)
    _statusCallback(PORTAL_START);
  if (_portalCb)
    _portalCb(); // Specialized Callback

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

String WiFiManager::now() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 0)) {
    return "";
  }
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

String WiFiManager::date() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 0)) {
    return "";
  }
  char buf[11];
  strftime(buf, sizeof(buf), "%Y-%m-%d", &timeinfo);
  return String(buf);
}

String WiFiManager::time() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 0)) {
    return "";
  }
  char buf[9];
  strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
  return String(buf);
}

time_t WiFiManager::getTimestamp() {
  time_t now;
  ::time(&now);
  return now;
}

bool WiFiManager::isTimeSynced() { return _timeSynced; }

void WiFiManager::initTime() {
  WM_LOG("[WiFiManager] Initializing Time Synchronization...");
  configTzTime(WM_TIME_ZONE, WM_NTP_SERVER);
  _lastTimeSync = millis();
}

void WiFiManager::checkTimeSync() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!_timeSynced || (millis() - _lastTimeSync > WM_TIME_SYNC_INTERVAL)) {
      struct tm timeinfo;
      if (getLocalTime(&timeinfo, 0)) {
        if (!_timeSynced) {
          WM_LOGF("[WiFiManager] Time Synced Successfully: %s\n",
                  now().c_str());
          _timeSynced = true;
        }
        _lastTimeSync = millis();
      }
    }
  }
}

bool WiFiManager::isConnected() { return WiFi.status() == WL_CONNECTED; }

String WiFiManager::getSSID() { return WiFi.SSID(); }

void WiFiManager::setupRoutes() {
  // Serve embedded index.html
  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", WM_HTML_INDEX);
  });

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
  const unsigned long AP_TIMEOUT = WM_AP_TIMEOUT; // From WM_Config.h

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

    // 3. Background Time Sync
    instance->checkTimeSync();

    // 4. Monitor WiFi Status & LED Management
    static bool lastConnected = false;
    static unsigned long lastBlink = 0;
    static bool ledState = false;

    bool currentlyConnected = (WiFi.status() == WL_CONNECTED);

    // Trigger Callbacks
    if (!instance->_portalRunning && currentlyConnected != lastConnected) {
      lastConnected = currentlyConnected;
      if (currentlyConnected) {
        instance->_isConnecting = false;
        if (instance->_statusCallback)
          instance->_statusCallback(CONNECTED);
        if (instance->_connectedCb)
          instance->_connectedCb();
      } else {
        instance->_isConnecting = true;
        if (instance->_statusCallback)
          instance->_statusCallback(DISCONNECTED);
        if (instance->_disconnectedCb)
          instance->_disconnectedCb();
      }
    }

    // LED Pattern Management
    if (instance->_ledPin != -1 && instance->_ledEnabled) {
      if (instance->_portalRunning) {
        // โหมด Portal: ติดค้าง
        digitalWrite(instance->_ledPin, instance->_ledInvert ? LOW : HIGH);
      } else if (instance->_isConnecting) {
        // กำลังหา/เชื่อมต่อ (จาก WM_Config.h)
        unsigned long cyclePos = millis() % WM_LED_CONNECTING_INT;
        bool shouldBeOn = (cyclePos < (unsigned long)instance->_ledPulseHold);
        digitalWrite(instance->_ledPin,
                     instance->_ledInvert ? !shouldBeOn : shouldBeOn);
      } else if (currentlyConnected) {
        // เชื่อมต่อสำเร็จ:
        // Sleep ON -> กระพริบช้ามาก, Sleep OFF -> กระพริบช้า (จาก WM_Config.h)
        int interval =
            instance->_sleepEnabled ? WM_LED_SLEEP_INT : WM_LED_CONNECTED_INT;
        unsigned long cyclePos = millis() % interval;
        bool shouldBeOn = (cyclePos < (unsigned long)instance->_ledPulseHold);
        digitalWrite(instance->_ledPin,
                     instance->_ledInvert ? !shouldBeOn : shouldBeOn);
      } else {
        // Sleep / AP Timeout / Idle: ไฟดับ
        digitalWrite(instance->_ledPin, instance->_ledInvert ? HIGH : LOW);
      }
    } else if (instance->_ledPin != -1 && !instance->_ledEnabled) {
      // Ensure LED is OFF if disabled during operation
      digitalWrite(instance->_ledPin, instance->_ledInvert ? HIGH : LOW);
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
