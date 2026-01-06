/**
 * Example: Basic Usage
 *
 * The simplest way to get WiFiManager running on your ESP32.
 * It handles connection, reconnections, and the setup portal automatically.
 */

#include <Arduino.h>
#include <WiFiManager.h>

void setup() {
  Serial.begin(115200);

  // One-liner to start everything:
  // 1. Tries to connect to known networks.
  // 2. Starts an AP named "ESP32-Setup-Portal" if connection fails.
  // 3. Manages LED status and NTP time in the background.
  wifiManager.begin("ESP32-Setup-Portal");

  Serial.println("WiFi Setup complete or running in background...");
  Serial.println("Current Time: " + wifiManager.now());
}

void loop() {
  // Your main code here.
  // WiFiManager runs on its own FreeRTOS task, so loop() stays clean!
}
