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
#define WM_LED_PORTAL_INTERVAL                                                 \
  150 // Rapid blink for Portal mode (not used with solid, but kept for ref)
#define WM_LED_CONNECTING_INT 500 // Fast pulse while connecting
#define WM_LED_CONNECTED_INT 3000 // Slow pulse heartbeat
#define WM_LED_SLEEP_INT 5000     // Very slow pulse for Low Power state

// --- Advanced Settings ---
#define WM_DEFAULT_RSSI_THRESHOLD -90
#define WM_DEFAULT_AP_TIMEOUT 300000 // 5 Minutes (Power saving)
#define WM_CONNECT_COOLDOWN_MS 1000  // Delay between connection attempts (ms)
#define WM_CONNECT_TIMEOUT_MS 15000  // Max time to wait for connection (ms)
#define WM_MAX_BOOT_RETRIES 3        // Number of full cycles to try before AP
#define WM_BOOT_RETRY_DELAY_MS 5000  // Rest time between full cycles (ms)

// --- RTC & NTP Settings ---
#define WM_NTP_SERVER "pool.ntp.org"
#define WM_TIME_ZONE "ICT-7"          // Bangkok, Thailand (UTC+7)
#define WM_TIME_SYNC_INTERVAL 3600000 // Resync every 1 hour (ms)

#endif // WM_CONFIG_H
