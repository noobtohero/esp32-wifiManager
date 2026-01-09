/**
 * Example: OTA Integration (Middleware Mode)
 *
 * This example shows how to combine WiFiManager with:
 * 1. ArduinoOTA (For flashing over WiFi)
 * 2. Standard WebServer Dashboard
 *
 * All networking runs in the background.
 */

#include <ArduinoOTA.h>
#include <WebServer.h>
#include <WiFiManager.h>


WebServer server(80);

void setup() {
  Serial.begin(115200);

  // 1. Setup WebServer Route
  server.on("/", []() {
    server.send(200, "text/html",
                "<h1>OTA Device is Running</h1><p>Time: " + wifiManager.now() +
                    "</p>");
  });

  // 2. Setup ArduinoOTA
  ArduinoOTA.setHostname("esp32-ota-device");
  ArduinoOTA.onStart([]() {
    String type =
        (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError(
      [](ota_error_t error) { Serial.printf("Error[%u]: ", error); });

  // 3. Register with WiFiManager
  // Note: ArduinoOTA.begin() should be called AFTER WiFi is connected.
  // We use the onConnected callback for this.
  wifiManager.setStatusLED()
      .useServer(&server)
      .onConnected([]() {
        ArduinoOTA.begin();
        Serial.println("[APP] OTA and WebServer ready!");
      })
      .begin("OTA-Setup-Portal");

  // Handle server start
  server.begin();
}

void loop() {
  // WiFiManager handles server.handleClient() in background task.
  // But ArduinoOTA.handle() still needs to be called.
  // You can put it here or create a separate task.
  ArduinoOTA.handle();

  delay(10);
}
