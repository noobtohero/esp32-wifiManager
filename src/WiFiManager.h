#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "WM_Config.h"
#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>
#include <functional>
#include <time.h>

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

  // Middleware Mode
  WiFiManager &useServer(WebServer *server);
  WebServer *getServer() { return _userServer ? _userServer : &_server; }

  // Callbacks & Types
  enum WiFiState { CONNECTED, DISCONNECTED, PORTAL_START, PORTAL_TIMEOUT };
  typedef std::function<void(WiFiState)> StatusCallback;
  typedef std::function<void()> SimpleCallback;
  typedef std::function<void(bool)> ConnectionCallback;

  // Core API
  bool begin(const char *apName = WM_DEFAULT_AP_NAME,
             const char *apPassword = WM_DEFAULT_AP_PASSWORD);
  bool begin(const char *apName, SimpleCallback onConnect);
  void resetSettings(bool restart = true);
  void clearSettings();

  // Status LED
  WiFiManager &setStatusLED(int pin = WM_DEFAULT_LED_PIN,
                            bool activeLow = WM_DEFAULT_LED_INVERT);
  WiFiManager &setLEDActiveTime(int ms) {
    _ledPulseHold = ms;
    return *this;
  }
  WiFiManager &enableStatusLED(bool enable) {
    _ledEnabled = enable;
    return *this;
  }
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
  void setSleep(bool enable);
  bool isConnected();
  String getSSID();
  void wakeUp();

  // Time Management
  String now();
  String date();
  String time();
  time_t getTimestamp();
  bool isTimeSynced();

private:
  // Internal methods
  static void wifiTask(void *pvParameters);
  void startAP();
  void startPortal();
  void stopPortal();
  void setupRoutes();
  void emitWiFiFound(int i);

  // Components
  WebServer _server;
  WebServer *_userServer = nullptr;
  DNSServer _dnsServer;
  ConnectionCallback _callback = nullptr;
  StatusCallback _statusCallback = nullptr;
  SimpleCallback _connectedCb = nullptr;
  SimpleCallback _disconnectedCb = nullptr;
  SimpleCallback _portalCb = nullptr;
  SimpleCallback _timeoutCb = nullptr;
  SleepCallback _sleepCallback = nullptr;

  // State
  bool _ledEnabled = true;
  String _apName;
  String _apPassword;
  bool _portalRunning = false;
  bool _sleepEnabled = true; // Default as set in constructor or begin
  bool _shouldRestart = false;
  bool _isConnecting = false;
  TaskHandle_t _taskHandle = nullptr;

  int _ledPin = -1;
  bool _ledInvert = false;
  int _ledPulseHold = WM_LED_PULSE_HOLD; // Default active time (ms)
  const byte DNS_PORT = WM_DNS_PORT;

  // Time Sync Members
  unsigned long _lastTimeSync = 0;
  bool _timeSynced = false;
  void initTime();
  void checkTimeSync();

  // Advanced Configuration

  // Scanning State
  int _scanChannel = 0;

  // Advanced Settings
  int _rssiThreshold = WM_DEFAULT_RSSI_THRESHOLD;
  unsigned long _apTimeout = WM_DEFAULT_AP_TIMEOUT;
  unsigned long _lastActivity = 0;
};

extern WiFiManager wifiManager;

#endif
