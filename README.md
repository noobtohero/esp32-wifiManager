# ESP32 WiFi Manager (Plug-and-Play) 🚀

**Latest Release:** `v0.2.1` (Hotfix: Begin LED, TimeSync)

A high-performance, non-blocking WiFi Manager for ESP32.

[**คู่มือสำหรับมือใหม่ (Thai Guide)**](GUIDE_TH.md)

---


## 📡 Overview

WiFiManager เป็นไลบรารีที่ครบครันสำหรับ ESP32 ที่จัดการการเชื่อมต่อ WiFi แบบอัตโนมัติ พร้อม Captive Portal สำหรับการตั้งค่า WiFi ผ่านเว็บเบราว์เซอร์ ออกแบบมาเพื่อความสะดวก ประหยัดพลังงาน และใช้งานง่าย เหมาะสำหรับอุปกรณ์ IoT ทุกประเภท

## ✨ Key Features

### 🔌 Smart WiFi Connection
- **Multi-Network Memory**: บันทึกและจัดการได้ถึง 3 เครือข่าย WiFi
- **Auto-Connect on Boot**: เชื่อมต่ออัตโนมัติเมื่อเปิดเครื่อง
- **Intelligent Retry System**: ลองเชื่อมต่อหลายรอบ (3 รอบ) ก่อนเปิด Portal
- **Priority Management**: จัดลำดับเครือข่ายตามความสำเร็จล่าสุด
- **Deduplication**: ป้องกันการบันทึก WiFi ซ้ำ
- **Credential Validation**: ทดสอบข้อมูล WiFi ก่อนบันทึกทุกครั้ง

### 🌐 Captive Portal
- **Universal Compatibility**: รองรับ iOS, Android, Windows, macOS, Linux
- **DNS Redirect**: Redirect ทุก domain มาที่ Portal อัตโนมัติ
- **Network Scanner**: สแกนและแสดง WiFi ที่พร้อมใช้งาน
- **RSSI Filtering**: กรองสัญญาณอ่อน (ค่าเริ่มต้น: -90 dBm)
- **Real-time Feedback**: แจ้งผลการเชื่อมต่อทันที
- **Auto-Shutdown**: ปิดอัตโนมัติเมื่อไม่มีการใช้งาน (5 นาที)

### ⚡ Power Management
- **Modem Sleep**: ลดการใช้พลังงานเมื่อเชื่อมต่อสำเร็จ
- **Smart Wake-up**: ปลุกระบบเมื่อจำเป็น
- **Configurable Timeout**: ตั้งค่าระยะเวลา Portal ได้
- **Zero-Power Standby**: ประหยัดพลังงานสูงสุดเมื่อไม่ใช้งาน

### 💡 LED Status Indicator
- **Visual Feedback**: LED แสดงสถานะการทำงาน
  - **Portal Mode**: กระพริบเร็ว (150ms intervals)
  - **Connecting**: Pulse ช้า (500ms intervals)
  - **Connected**: Heartbeat ช้า (3 วินาที)
  - **Sleep Mode**: Pulse ช้ามาก (5 วินาที)
- **Flexible Configuration**: รองรับ Active-High และ Active-Low
- **Enable/Disable**: เปิด-ปิดได้ตามต้องการ

### ⏰ Time Synchronization
- **Auto NTP Sync**: ซิงค์เวลาอัตโนมัติเมื่อเชื่อมต่อ
- **Timezone Support**: ตั้งค่า Timezone ได้ (เริ่มต้น: ICT-7)
- **Periodic Resync**: ซิงค์ซ้ำทุก 1 ชั่วโมง
- **Utility Functions**:
  - `now()` → "2024-01-15 14:30:00"
  - `date()` → "2024-01-15"
  - `time()` → "14:30:00"
  - `getTimestamp()` → Unix timestamp
  - `isTimeSynced()` → ตรวจสอบสถานะ

### 🎯 Event System
- **Rich Callbacks**: แจ้งเตือนเหตุการณ์ต่างๆ
  - Connected / Disconnected
  - Portal Start / Timeout
  - Sleep Mode Changes
- **Non-blocking**: ทำงานด้วย FreeRTOS Task
- **Thread-safe**: ปลอดภัยสำหรับ multi-threading

## 🚀 Quick Start

### platformio.ini

```cpp
lib_deps =
    https://github.com/your-username/ESP32-WiFiManager.git

build_flags = 
    -DDEBUG_MODE ; enable debug mode, comment in production
```

### Basic Usage

```cpp
#include "WiFiManager.h"


void setup() {
    // เริ่มต้นแบบง่าย
    wifiManager.begin("MyESP32");
}

void loop() {
    // ไม่ต้องทำอะไร - จัดการอัตโนมัติ
}
```

### With Password

```cpp
void setup() {
    wifiManager.begin("MyESP32", "secret123");
}
```

### With Callback

```cpp
void onWiFiConnected() {
    Serial.println("WiFi Connected!");
    Serial.println("IP: " + WiFi.localIP().toString());
}

void setup() {
    wifiManager.begin("MyESP32", onWiFiConnected);
}
```

## 📚 API Reference

### Initialization

```cpp
// เริ่มต้นด้วย AP name
bool begin(const char* apName);

// เริ่มต้นพร้อม password
bool begin(const char* apName, const char* apPassword);

// เริ่มต้นพร้อม callback
bool begin(const char* apName, SimpleCallback onConnect);
```

### LED Configuration

```cpp
// ตั้งค่า LED pin (Active-High)
WiFiManager& setStatusLED(int pin);

// Active-Low (เช่น NodeMCU built-in LED)
WiFiManager& setStatusLED(int pin, bool activeLow);

// เปิด/ปิด LED
void enableLED(bool enable);
```

### Event Callbacks

```cpp
// เชื่อมต่อสำเร็จ
WiFiManager& onConnect(SimpleCallback callback);

// ขาดการเชื่อมต่อ
WiFiManager& onDisconnect(SimpleCallback callback);

// Portal เปิด
WiFiManager& onPortalStart(SimpleCallback callback);

// Portal timeout
WiFiManager& onPortalTimeout(SimpleCallback callback);

// Sleep mode เปลี่ยน
WiFiManager& onSleep(SleepCallback callback);

// สถานะเปลี่ยน (รวมทั้งหมด)
WiFiManager& onStatus(StatusCallback callback);
```

### Settings Management

```cpp
// ล้างการตั้งค่า WiFi
void clearSettings();

// ล้างและรีสตาร์ท
void resetSettings(bool restart = true);
```

### Power Management

```cpp
// ตั้งค่า Modem Sleep
void setSleep(bool enable);

// ตั้งค่า Portal timeout (มิลลิวินาที)
WiFiManager& setTimeout(unsigned long ms);

// ปลุกระบบ (เปิด Portal)
void wakeUp();

// หยุด Portal
void stopPortal();
```

### Status & Info

```cpp
// ตรวจสอบการเชื่อมต่อ
bool isConnected();

// ดึง SSID ปัจจุบัน
String getSSID();

// ตรวจสอบ Portal
bool isPortalRunning();

// ตรวจสอบการซิงค์เวลา
bool isTimeSynced();
```

### Time Functions

```cpp
// วันที่และเวลา
String now();  // "2024-01-15 14:30:00"

// วันที่
String date();  // "2024-01-15"

// เวลา
String time();  // "14:30:00"

// Unix timestamp
time_t getTimestamp();
```

### Advanced

```cpp
// ใช้ WebServer ของคุณเอง
WiFiManager& useServer(WebServer* server);

// ตั้งค่า RSSI threshold
WiFiManager& setRSSIThreshold(int rssi);
```

## ⚙️ Configuration

### Default Values (WM_Config.h)

```cpp
// WiFi Settings
#define WM_DEFAULT_AP_NAME "ESP32-Smart-Portal"
#define WM_DEFAULT_AP_PASSWORD nullptr
#define WM_DNS_PORT 53

// Hardware
#define WM_DEFAULT_LED_PIN LED_BUILTIN
#define WM_DEFAULT_LED_INVERT false

// Timing & Intervals (milliseconds)
#define WM_LED_PULSE_HOLD 150        // LED ON duration
#define WM_LED_PORTAL_INTERVAL 150   // Portal blink speed
#define WM_LED_CONNECTING_INT 500    // Connecting pulse
#define WM_LED_CONNECTED_INT 3000    // Connected heartbeat
#define WM_LED_SLEEP_INT 5000        // Sleep mode pulse

// Connection Settings
#define WM_CONNECT_TIMEOUT_MS 15000  // 15 seconds
#define WM_CONNECT_COOLDOWN_MS 1000  // 1 second between attempts
#define WM_MAX_BOOT_RETRIES 3        // Number of retry cycles
#define WM_BOOT_RETRY_DELAY_MS 5000  // 5 seconds between cycles

// Portal Settings
#define WM_DEFAULT_AP_TIMEOUT 300000 // 5 minutes
#define WM_DEFAULT_RSSI_THRESHOLD -90 // dBm

// Time Sync
#define WM_NTP_SERVER "pool.ntp.org"
#define WM_TIME_ZONE "ICT-7"          // Bangkok (UTC+7)
#define WM_TIME_SYNC_INTERVAL 3600000 // 1 hour
```

### Customization

แก้ไขค่าใน `WM_Config.h` หรือตั้งค่าผ่าน API:

```cpp
wifiManager
    .setTimeout(600000)        // Portal timeout 10 นาที
    .setRSSIThreshold(-85)     // สัญญาณขั้นต่ำ -85 dBm
    .setStatusLED(LED_PIN);
```

## 🔄 How It Works

### Boot Sequence

```
1. START
   ↓
2. ลองเชื่อมต่อ WiFi ที่บันทึกไว้
   ├─ เครือข่าย 1 (timeout: 15s)
   ├─ เครือข่าย 2 (timeout: 15s)
   └─ เครือข่าย 3 (timeout: 15s)
   ↓
3. ถ้าล้มเหลว รอ 5 วินาที แล้วลองรอบใหม่
   ↓
4. ลองครบ 3 รอบแล้ว?
   ├─ ใช่ → เปิด AP + Portal
   └─ ไม่ → กลับไปข้อ 2
   ↓
5. PORTAL MODE
   ├─ ผู้ใช้เลือก WiFi
   ├─ ใส่ password
   ├─ ทดสอบการเชื่อมต่อ
   └─ บันทึก (ถ้าสำเร็จ) → รีสตาร์ท
   ↓
6. CONNECTED
   └─ Enable Modem Sleep → ประหยัดพลังงาน
```

### Portal Workflow

```
1. ผู้ใช้เชื่อมต่อกับ AP "ESP32-Smart-Portal"
   ↓
2. DNS Redirect → http://192.168.4.1
   ↓
3. เว็บแสดงรายการ WiFi ที่สแกนได้
   ↓
4. เลือก WiFi + ใส่ password
   ↓
5. ทดสอบการเชื่อมต่อทันที
   ├─ สำเร็จ → บันทึก → รีสตาร์ท
   └─ ล้มเหลว → แจ้งเตือน (ไม่บันทึก)
```

## 💡 Examples

### Complete Example with All Features

```cpp
#include "WiFiManager.h"

WiFiManager wifiManager;

void onConnect() {
    Serial.println("✓ WiFi Connected!");
    Serial.println("IP: " + WiFi.localIP().toString());
    Serial.println("Time: " + wifiManager.now());
}

void onDisconnect() {
    Serial.println("✗ WiFi Disconnected");
}

void onPortalStart() {
    Serial.println("⚡ Portal Started");
    Serial.println("Connect to: ESP32-Smart-Portal");
}

void onSleep(bool enabled) {
    Serial.println(enabled ? "💤 Sleep Enabled" : "👁 Sleep Disabled");
}

void setup() {
    Serial.begin(115200);
    
    // ตั้งค่า LED
    wifiManager.setStatusLED(2, false);  // GPIO2, Active-High
    
    // ตั้งค่า Callbacks
    wifiManager
        .onConnect(onConnect)
        .onDisconnect(onDisconnect)
        .onPortalStart(onPortalStart)
        .onSleep(onSleep)
        .setTimeout(600000);  // 10 minutes
    
    // เริ่มต้น
    wifiManager.begin("MySmartDevice");
}

void loop() {
    if (wifiManager.isConnected()) {
        // ทำงานปกติ
        Serial.println("Working... " + wifiManager.time());
        delay(10000);
    }
}
```

### Reset Button Example

```cpp
#define RESET_BUTTON 0  // Boot button

void setup() {
    pinMode(RESET_BUTTON, INPUT_PULLUP);
    wifiManager.begin("MyDevice");
}

void loop() {
    // กดปุ่ม 3 วินาทีเพื่อรีเซ็ต
    if (digitalRead(RESET_BUTTON) == LOW) {
        delay(3000);
        if (digitalRead(RESET_BUTTON) == LOW) {
            Serial.println("Resetting WiFi...");
            wifiManager.resetSettings(true);
        }
    }
    delay(100);
}
```

### Time-based Actions

```cpp
void loop() {
    if (wifiManager.isTimeSynced()) {
        String currentTime = wifiManager.time();
        
        // ทำงานเฉพาะเวลา 08:00:00
        if (currentTime.startsWith("08:00:")) {
            doMorningTask();
        }
    }
    delay(1000);
}
```

## 🔒 Security Features

- ✅ Password validation ก่อนบันทึก
- ✅ ไม่บันทึกข้อมูลที่ผิดพลาด
- ✅ รองรับ AP ที่มี password
- ✅ RSSI threshold (หลีกเลี่ยงสัญญาณอ่อน)
- ✅ Timeout protection
- ✅ Memory-safe (NVS storage)

## 📱 Compatibility

| Platform | Status | Notes |
|----------|--------|-------|
| **iOS** | ✅ Full | Captive Portal auto-detect |
| **Android** | ✅ Full | Captive Portal auto-detect |
| **Windows** | ✅ Full | Manual navigation required |
| **macOS** | ✅ Full | Captive Portal auto-detect |
| **Linux** | ✅ Full | Manual navigation required |

## 🛠️ Dependencies

```
- ESP32 Arduino Core (>= 2.0.0)
- Preferences (Built-in)
- WebServer (Built-in)
- DNSServer (Built-in)
- WiFi (Built-in)
```

## 📊 Performance

- **Boot Time**: 1-45 วินาที (ขึ้นกับจำนวน retry)
- **Portal Response**: < 5ms
- **Memory Usage**: ~15KB RAM
- **Power Consumption**:
  - Active: ~160-260 mA
  - Sleep: ~20-30 mA
  - Deep Sleep Ready: Yes (manual)

## 🐛 Troubleshooting

### Portal ไม่เปิดอัตโนมัติ
- ตรวจสอบว่าไม่มี WiFi ที่บันทึกไว้สามารถเชื่อมต่อได้
- ลอง `wifiManager.clearSettings()` แล้วรีสตาร์ท

### เชื่อมต่อไม่สำเร็จ
- ตรวจสอบสัญญาณ WiFi (ต้องมากกว่า -90 dBm)
- ตรวจสอบ password
- ตรวจสอบ log ใน Serial Monitor

### LED ไม่ทำงาน
- ตรวจสอบ pin number
- ลอง `setStatusLED(pin, true)` สำหรับ Active-Low
- ตรวจสอบว่า LED ไม่ถูก disable

### เวลาไม่ถูกต้อง
- ตรวจสอบการเชื่อมต่อ Internet
- แก้ไข `WM_TIME_ZONE` ใน Config
- ตรวจสอบด้วย `isTimeSynced()`



---

**Tips**: 
- ใช้ Serial Monitor (115200 baud) เพื่อดู debug logs
- Portal IP เริ่มต้นคือ `192.168.4.1`
- กดปุ่ม Boot ค้าง 3 วินาทีเพื่อรีเซ็ต (ถ้าเขียนโค้ดไว้)
- ข้อมูล WiFi เก็บใน NVS ไม่หายแม้ reset code

### 🧠 Middleware Mode (ใช้ WebServer ร่วมกับ Dashboard)

หากคุณมี `WebServer` ส่วนตัวสำหรับทำ Dashboard หรือระบบ OTA คุณสามารถแชร์ Port 80 ร่วมกับ WiFiManager ได้ เพื่อประหยัด RAM และป้องกัน Port ชนกัน

```cpp
WebServer myServer(80);

void setup() {
    myServer.on("/hello", []() { myServer.send(200, "text/plain", "Hello!"); });

    wifiManager
        .useServer(&myServer) // ลงทะเบียน Server ของคุณ
        .begin("My-Device-Portal");
    
    myServer.begin();
}
```
WiFiManager จะเรียกคำสั่ง `myServer.handleClient()` ให้เองโดยอัตโนมัติภายใน FreeRTOS Task ของมัน ดังนั้นคุณไม่จำเป็นต้องใส่ไว้ใน `loop()` ของคุณครับ

---

### ⚠️ Note for Developers
When editing frontend source files in the `data/` folder, you must run:
`python generate_assets.py` 
to update the embedded header file (`WebAssets.h`).

---
## 📄 License

MIT License - ใช้งานได้อย่างอิสระ

## 🙏 Credits


Developed for ESP32 IoT Projects with ❤️

Developed by **Antigravity AI (Google Deepmind)** 🧬

Managed by **NoobToHERO** 🛠️
