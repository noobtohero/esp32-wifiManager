/**
 * Example: Dashboard Integration (Middleware Mode)
 *
 * This example demonstrates how to use WiFiManager as a middleware
 * with your own standard WebServer dashboard.
 * Both the WiFi Setup Portal and your Dashboard will run on Port 80.
 *
 * 1. WiFiManager handles SSID/Pass configuration when disconnected.
 * 2. Your Dashboard handles user requests once connected.
 *
 * This file is for the Arduino IDE.
 */

#include <WebServer.h>
#include <WiFiManager.h>


// 1. Create your own server instance
WebServer myServer(80);

void setup() {
  Serial.begin(115200);

  // 2. Setup your Dashboard Routes
  myServer.on("/", HTTP_GET, []() {
    String html = "<html><body style='font-family:sans-serif; "
                  "text-align:center; padding:50px;'>";
    html += "<h1>🚀 My ESP32 Dashboard</h1>";
    html += "<p>Current Time: <b>" + wifiManager.now() + "</b></p>";
    html += "<hr>";
    html += "<p>Device is healthy and connected.</p>";
    html += "</body></html>";
    myServer.send(200, "text/html", html);
  });

  // 3. Register your server with WiFiManager
  // This makes WiFiManager use YOUR server instance instead of starting its
  // own.
  wifiManager
      .setStatusLED()       // Use built-in LED
      .useServer(&myServer) // <--- CRITICAL: Share the server instance
      .onConnected([]() {
        Serial.println("[APP] Dashboard is now online at http://" +
                       WiFi.localIP().toString());
      })
      .begin("Config-My-Device");

  // 4. Start your server (Must be done after wifiManager.begin() or if you want
  // it to run always) WiFiManager will call handleClient() internally in its
  // FreeRTOS task.
  myServer.begin();
}

void loop() {
  // Main loop remains empty.
  // WiFiManager's FreeRTOS task handles:
  // - DNS Server
  // - Web Server requests (myServer.handleClient())
  // - WiFi Status & Reconnections
}
