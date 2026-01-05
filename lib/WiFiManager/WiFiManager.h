#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <functional>

// Fallback for boards that don't define LED_BUILTIN (Commonly pin 2 on ESP32)
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// Debug Macros
#ifdef DEBUG_MODE
#define WM_LOG(x) Serial.println(x)
#define WM_LOGF(x, ...) Serial.printf(x, ##__VA_ARGS__)
#else
#define WM_LOG(x)
#define WM_LOGF(x, ...)
#endif

class WiFiManager {
public:
  WiFiManager();
  ~WiFiManager();

  // Callbacks & Types
  enum WiFiState { CONNECTED, DISCONNECTED, PORTAL_START, PORTAL_TIMEOUT };
  typedef std::function<void(WiFiState)> StatusCallback;
  typedef std::function<void()> SimpleCallback;
  typedef std::function<void(bool)> ConnectionCallback;

  // Core API
  bool begin(const char *apName = "ESP32-Portal",
             const char *apPassword = nullptr);
  bool begin(const char *apName, SimpleCallback onConnect);
  void resetSettings();

  // Status LED (Default to Pin 2 for ESP32 DevKits)
  WiFiManager &setStatusLED(int pin = LED_BUILTIN, bool activeLow = false);
  bool isSleepEnabled() { return _sleepEnabled; }

  // Callbacks Setters (Fluent API)
  typedef std::function<void(bool)> SleepCallback;

  WiFiManager &onStatusChange(StatusCallback cb) {
    _statusCallback = cb;
    return *this;
  }
  WiFiManager &onSleepChange(SleepCallback cb) {
    _sleepCallback = cb;
    return *this;
  }
  WiFiManager &onConnected(SimpleCallback cb) {
    _connectedCb = cb;
    return *this;
  }
  WiFiManager &onDisconnected(SimpleCallback cb) {
    _disconnectedCb = cb;
    return *this;
  }
  WiFiManager &onPortal(SimpleCallback cb) {
    _portalCb = cb;
    return *this;
  }
  WiFiManager &onTimeout(SimpleCallback cb) {
    _timeoutCb = cb;
    return *this;
  }
  WiFiManager &onConnectionResult(ConnectionCallback cb) {
    _callback = cb;
    return *this;
  }

  // Status & Settings
  bool isConnected();
  String getSSID();
  void setSleep(bool enable);

private:
  // Internal methods
  static void wifiTask(void *pvParameters);
  void startAP();
  void startPortal();
  void stopPortal();
  void setupRoutes();
  void emitWiFiFound(int i);

  // Components
  AsyncWebServer _server;
  AsyncEventSource _events;
  DNSServer _dnsServer;
  ConnectionCallback _callback = nullptr;
  StatusCallback _statusCallback = nullptr;
  SimpleCallback _connectedCb = nullptr;
  SimpleCallback _disconnectedCb = nullptr;
  SimpleCallback _portalCb = nullptr;
  SimpleCallback _timeoutCb = nullptr;
  SleepCallback _sleepCallback = nullptr;

  // State
  String _apName;
  String _apPassword;
  bool _portalRunning = false;
  bool _sleepEnabled = true; // Default as set in constructor or begin
  bool _shouldRestart = false;
  TaskHandle_t _taskHandle = nullptr;

  int _ledPin = -1;
  bool _ledInvert = false;
  const byte DNS_PORT = 53;
};

extern WiFiManager wifiManager;

#endif
