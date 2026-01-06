/**
 * Example: Dashboard Integration (Middleware Mode)
 *
 * This example demonstrates how to use WiFiManager as a middleware
 * with your own AsyncWebServer dashboard.
 * Both the WiFi Setup Portal and your Dashboard will run on Port 80.
 *
 * 1. WiFiManager handles SSID/Pass configuration when disconnected.
 * 2. Your Dashboard handles user requests once connected.
 */

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WiFiManager.h>


// Create your own server instance
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // 1. Setup your Dashboard Routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<h1>My Device Dashboard</h1>";
    html += "<p>Current Time: " + wifiManager.now() + "</p>";
    html += "<p><a href='/config_wifi'>Setup WiFi</a> (Direct Link)</p>";
    request->send(200, "text/html", html);
  });

  // Optional: Add a link to force the portal if needed
  server.on("/config_wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "WiFi Portal is being forced...");
    // wifiManager.startPortal(); // You could trigger this manually
  });

  // 2. Register your server with WiFiManager
  // This makes WiFiManager act as a 'plugin' for your server
  wifiManager
      .setStatusLED()     // Use built-in LED
      .useServer(&server) // <--- CRITICAL: Share the server
      .onConnected([]() {
        Serial.println("Dashboard is now online at http://" +
                       WiFi.localIP().toString());
      })
      .begin("Config-My-Device");

  // 3. Start your server
  server.begin();
}

void loop() {
  // Nothing needed here, everything runs in FreeRTOS background tasks
}
