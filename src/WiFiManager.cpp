#include "WiFiManager.h"
#include "WebAssets.h"
#include <Preferences.h>
#include <WiFi.h>
#include <functional>
#include <time.h>

WiFiManager wifiManager;

WiFiManager::WiFiManager()
    : _server(80), _portalRunning(false), _shouldRestart(false),
      _taskHandle(nullptr) {}

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

  initTime();

  if (!_taskHandle) {
    xTaskCreate(wifiTask, "wifi_task", 4096, this, 1, &_taskHandle);
  }

  // Try Auto-connecting to Last 3 Networks (Stored as separate keys)
  Preferences prefs;
  prefs.begin("wifi-manager", true); // Read-only

  for (int i = 0; i < 3; i++) {
    String keySsid = "s" + String(i);
    String keyPass = "p" + String(i);

    if (prefs.isKey(keySsid.c_str())) {
      String ssid = prefs.getString(keySsid.c_str(), "");
      String pass = prefs.getString(keyPass.c_str(), "");

      if (ssid.length() > 0) {
        WiFi.mode(WIFI_STA);
        WM_LOGF("[WiFiManager] Trying network %d: %s\n", i, ssid.c_str());
        WiFi.begin(ssid.c_str(), pass.c_str());

        unsigned long startAttemptTime = millis();
        while (millis() - startAttemptTime < 10000) {
          if (WiFi.status() == WL_CONNECTED)
            break;
          vTaskDelay(pdMS_TO_TICKS(500));
          WM_LOGF(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
          WM_LOG("\n[WiFiManager] Connected successfully!");
          this->setSleep(true);
          _isConnecting = false;

          // Clear Error Flag
          prefs.remove("conn_error");
          prefs.end();

          if (_statusCallback)
            _statusCallback(CONNECTED);
          if (_connectedCb)
            _connectedCb();
          if (_callback)
            _callback(true);
          return true;
        }
        WM_LOG("\n[WiFiManager] Failed to connect to " + ssid);
      }
    }
  }
  prefs.end();

  _isConnecting = false;
  startAP();
  startPortal();

  if (_statusCallback)
    _statusCallback(PORTAL_START);
  if (_portalCb)
    _portalCb();

  // Store Error Flag
  prefs.begin("wifi-manager", false);
  prefs.putBool("conn_error", true);
  prefs.end();

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
  WM_LOG("[WiFiManager] Configuring AP...");

  // 1. Clean up any previous STA connection attempt FIRST
  WM_LOG("[WiFiManager] Disconnecting STA for stability...");
  WiFi.disconnect();
  delay(100);

  // 2. Set Mode to AP_STA (required for scanning)
  WM_LOG("[WiFiManager] Setting Mode: AP+STA");
  WiFi.mode(WIFI_AP_STA);

  WM_LOGF("[WiFiManager] Starting SoftAP: %s\n", _apName.c_str());

  WiFi.softAP(_apName.c_str(),
              _apPassword.length() > 0 ? _apPassword.c_str() : nullptr);

  // Define Standard IP for Portal (192.168.4.1)
  IPAddress IP(192, 168, 4, 1);
  IPAddress NMask(255, 255, 255, 0);

  // Explicitly configure AP to ensure DHCP Server gives out this IP as DNS
  WiFi.softAPConfig(IP, IP, NMask);

  WM_LOGF("[WiFiManager] AP Active: %s\n", _apName.c_str());
  WM_LOGF("[WiFiManager] Portal IP: ");
  WM_LOG(WiFi.softAPIP());

  _dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
}

WiFiManager &WiFiManager::useServer(WebServer *server) {
  _userServer = server;
  return *this;
}

void WiFiManager::stopPortal() {
  if (_portalRunning) {
    _dnsServer.stop();
    _server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    _portalRunning = false;
    if (_statusCallback)
      _statusCallback(PORTAL_TIMEOUT);
    if (_timeoutCb)
      _timeoutCb();
    if (_connectedCb)
      _connectedCb();
    WM_LOG("[WiFiManager] Portal & AP Stopped.");
    WiFi.setSleep(true);
  }
}

void WiFiManager::startPortal() {
  WM_LOG("[WiFiManager] Starting Portal in Standalone Mode...");

  // บังคับปิดประหยัดพลังงานเพื่อให้ iPhone เชื่อมต่อได้เสถียร
  WiFi.setSleep(false);

  setupRoutes();
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

void WiFiManager::wakeUp() {
  if (!_portalRunning && !isConnected()) {
    WM_LOG("[WiFiManager] Waking up...");
    WiFi.setSleep(false);
    startAP();
    startPortal();
    _lastActivity = millis();
  } else {
    // Reset timer even if running
    _lastActivity = millis();
  }
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
  _server.on("/", HTTP_GET,
             [this]() { _server.send(200, "text/html", WM_HTML_INDEX); });

  _server.on("/save", HTTP_POST, [this]() {
    if (_server.hasArg("ssid")) {
      String s = _server.arg("ssid");
      String p = _server.hasArg("password") ? _server.arg("password") : "";

      // --- Verify Credentials (Instant Feedback) ---
      WM_LOGF("[WiFiManager] Testing connection to: %s\n", s.c_str());

      // Keep AP alive, but connect STA
      WiFi.begin(s.c_str(), p.c_str());

      unsigned long startAttempt = millis();
      bool connected = false;

      // Wait up to 15 seconds
      WM_LOG("[WiFiManager] Waiting for connection result...");
      while (millis() - startAttempt < 15000) {
        if (WiFi.status() == WL_CONNECTED) {
          WM_LOG("[WiFiManager] Connection Successful!");
          connected = true;
          break;
        }
        // Keep Server Alive!
        _server.handleClient();
        if (_userServer)
          _userServer->handleClient();
        _dnsServer.processNextRequest();
        yield();
      }

      if (connected) {
        // Success! Save and Restart
        Preferences prefs;
        prefs.begin("wifi-manager", false);

        // Shift networks (1->2, 0->1)
        for (int i = 1; i >= 0; i--) {
          String keyS = "s" + String(i);
          String keyP = "p" + String(i);
          if (prefs.isKey(keyS.c_str())) {
            String ssid = prefs.getString(keyS.c_str(), "");
            String pass = prefs.getString(keyP.c_str(), "");
            if (ssid.length() > 0) {
              prefs.putString(("s" + String(i + 1)).c_str(), ssid.c_str());
              prefs.putString(("p" + String(i + 1)).c_str(), pass.c_str());
            }
          }
        }

        // Save New
        prefs.putString("s0", s.c_str());
        prefs.putString("p0", p.c_str());

        // Clear any previous error
        prefs.remove("conn_error");
        prefs.end();

        _server.send(200, "application/json", "{\"status\":\"connected\"}");
        _shouldRestart = true;
      } else {
        // Failed! Do NOT save, Do NOT restart
        WM_LOG("[WiFiManager] Connection Failed! Wrong password?");
        _server.send(200, "application/json",
                     "{\"status\":\"failed\", \"reason\":\"auth_error\"}");
      }
    } else {
      _server.send(400, "text/plain", "Missing SSID");
    }
  });

  _server.on("/hotspot-detect.html", HTTP_GET, [this]() {
    _server.sendHeader("Location", "/", true);
    _server.send(302, "text/plain", "");
  });

  _server.on("/error", HTTP_GET, [this]() {
    Preferences prefs;
    prefs.begin("wifi-manager", false); // Read-Write mode
    bool err = prefs.getBool("conn_error", false);
    if (err) {
      prefs.remove("conn_error"); // Consume the error
    }
    prefs.end();
    _server.send(200, "text/plain", err ? "true" : "false");
  });

  _server.on("/success.txt", HTTP_GET,
             [this]() { _server.send(200, "text/plain", "success"); });
  _server.on("/ncsi.txt", HTTP_GET, [this]() {
    _server.sendHeader("Location", "/", true);
    _server.send(302, "text/plain", "");
  });
  _server.on("/generate_204", HTTP_GET, [this]() {
    _server.sendHeader("Location", "/", true);
    _server.send(302, "text/plain", "");
  });
  _server.on("/fwlink", HTTP_GET, [this]() {
    _server.sendHeader("Location", "/", true);
    _server.send(302, "text/plain", "");
  });
  _server.on("/canonical.html", HTTP_GET, [this]() {
    _server.sendHeader("Location", "/", true);
    _server.send(302, "text/plain", "");
  });
  // Handle Apple captive portal checks
  _server.on("/library/test/success.html", HTTP_GET, [this]() {
    _server.send(200, "text/html",
                 "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</"
                 "BODY></HTML>");
  });

  _server.on("/list", HTTP_GET, [this]() {
    int n = WiFi.scanComplete();
    String json = "[";

    if (n == WIFI_SCAN_FAILED || n == -2) {
      // Start Scan (Async, All Channels)
      WM_LOG("[WiFiManager] Starting background scan...");
      WiFi.scanNetworks(true);
    } else if (n >= 0) {
      // We have results
      WM_LOGF("[WiFiManager] Scan Completed. Found %d networks.\n", n);
      bool first = true;
      for (int i = 0; i < n; ++i) {
        if (WiFi.RSSI(i) < _rssiThreshold)
          continue; // Skip weak networks

        if (!first)
          json += ",";
        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json +=
            "\"secure\":" +
            String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false");
        json += "}";
        first = false;
      }

      // Reset Activity Timer
      _lastActivity = millis();
      WiFi.scanDelete();

      // Trigger Next Scan (PASSIVE MODE)
      // We do NOT start a new scan immediately here to prevent RF
      // starvation/WDT resets. The next client poll (1.5s later) will find
      // scanComplete() == -2 and trigger it then.
    }

    json += "]";
    _server.send(200, "application/json", json);
  });

  _server.onNotFound([this]() {
    String uri = _server.uri();
    // ดักจับไฟล์ Icon และไฟล์อื่นๆ ที่ไม่จำเป็น
    if (uri.endsWith(".ico") || uri.endsWith(".png")) {
      _server.send(404, "text/plain", "");
      return;
    }
    WM_LOGF("[WebServer] Redirecting %s to Portal\n", uri.c_str());
    _server.sendHeader("Location", "/", true);
    _server.send(302, "text/plain", "");
  });
}

void WiFiManager::emitWiFiFound(int i) {
  // Logic moved to /list route for polling
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

    // Auto-AP Timeout (Configurable)
    if (instance->_portalRunning &&
        (millis() - instance->_lastActivity > instance->_apTimeout)) {
      if (WiFi.softAPgetStationNum() == 0) {
        WM_LOG("[WiFiManager] AP Timeout - No activity. Shutting down.");
        instance->stopPortal();
        // Force Low Energy
        WiFi.setSleep(true);
      } else {
        instance->_lastActivity = millis(); // Reset if clients connected
      }
    }

    // 1. Process Requests
    if (instance->_portalRunning) {
      instance->_server.handleClient();
      if (instance->_userServer)
        instance->_userServer->handleClient();
      instance->_dnsServer.processNextRequest();
    }

    // 2. Smooth Background Scanning (Non-blocking)
    // REMOVED: Scanning is now driven by /list endpoint on demand.

    // 3. Background Time Sync
    instance->checkTimeSync();

    // 4. Monitor WiFi Status & LED Management
    static bool lastConnected = false;
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
        // Portal Mode: Rapid Blink (Indicating Hotspot Active)
        int interval = WM_LED_PORTAL_INTERVAL;
        digitalWrite(instance->_ledPin,
                     (millis() / interval) % 2 == 0
                         ? (instance->_ledInvert ? LOW : HIGH)
                         : (instance->_ledInvert ? HIGH : LOW));
      } else if (instance->_isConnecting) {
        unsigned long cyclePos = millis() % WM_LED_CONNECTING_INT;
        bool shouldBeOn = (cyclePos < (unsigned long)instance->_ledPulseHold);
        digitalWrite(instance->_ledPin,
                     instance->_ledInvert ? !shouldBeOn : shouldBeOn);
      } else if (currentlyConnected) {
        int interval =
            instance->_sleepEnabled ? WM_LED_SLEEP_INT : WM_LED_CONNECTED_INT;
        unsigned long cyclePos = millis() % interval;
        bool shouldBeOn = (cyclePos < (unsigned long)instance->_ledPulseHold);
        digitalWrite(instance->_ledPin,
                     instance->_ledInvert ? !shouldBeOn : shouldBeOn);
      } else {
        digitalWrite(instance->_ledPin, instance->_ledInvert ? HIGH : LOW);
      }
    } else if (instance->_ledPin != -1 && !instance->_ledEnabled) {
      digitalWrite(instance->_ledPin, instance->_ledInvert ? HIGH : LOW);
    }

    // Critical: Reduce delay to responsive 1-5ms
    // iOS queries are rapid; 50ms is too slow for some handshake bursts.
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
