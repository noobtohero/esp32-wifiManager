#ifndef WM_CONFIG_H
#define WM_CONFIG_H

#include <Arduino.h>

/**
 * ESP32 WiFi Manager Central Configuration
 */

// --- WiFi Settings ---
#define WM_DEFAULT_AP_NAME "ESP32-Smart-Portal"
#define WM_DEFAULT_AP_PASSWORD nullptr
#define WM_DNS_PORT 53

// --- Hardware Settings ---
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#define WM_DEFAULT_LED_PIN LED_BUILTIN
#define WM_DEFAULT_LED_INVERT false

// --- Timing & Intervals (ms) ---
#define WM_LED_PULSE_HOLD 150 // Duration LED stays ON during pulse
#define WM_LED_PORTAL_INTERVAL 150 // Rapid blink for Portal mode (not used with solid, but kept for ref)
#define WM_LED_CONNECTING_INT 500 // Fast pulse while connecting
#define WM_LED_CONNECTED_INT 3000 // Slow pulse heartbeat
#define WM_LED_SLEEP_INT 5000     // Very slow pulse for Low Power state
#define WM_AP_TIMEOUT 300000  // 5 Minutes (Power saving)

#endif // WM_CONFIG_H
