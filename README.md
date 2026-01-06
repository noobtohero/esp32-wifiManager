# ESP32 WiFi Manager (Plug-and-Play) 🚀

[English](#english) | [ภาษาไทย](#ภาษาไทย)

---

## English

A WiFi management library for ESP32 designed to be **Easy to use (Eazy)**, **Fast**, **Low Energy**, and **Non-blocking**, powered by FreeRTOS.

### ✨ Features

- **Plug-and-Play:** One-liner setup with `wifiManager.begin()`.
- **Smooth Scan:** Real-time WiFi scanning where results appear progressively (Fomantic-UI style).
- **Captive Portal:** Automatic configuration page popup upon connection (DNS Redirect supported).
- **Multi-Network Auto-connect:** Automatically remembers and reconnects to up to the 3 last successful networks.
- **Low Energy:** Automatic Modem Sleep and AP auto-shutdown when idle (AP Timeout).
- **Non-blocking FreeRTOS:** Background WiFi management that doesn't freeze your main loop.

### 🛠 Installation

Add this line to your `platformio.ini`:

```ini
lib_deps =
    https://github.com/noobtohero/esp32-wifiManager.git
```

**Zero-Config!** The library automatically handles all dependencies (AsyncWebServer, ArduinoJson) and embeds web assets into the binary.

### 📖 Quick Start

You can use the library in two styles:

#### 1. 🚀 Simplified API (Recommended)

Chain methods together for clean setup:

```cpp
void setup() {
    wifiManager
        .setStatusLED()        // (Optional) Auto-status LED using built-in pin
        .onConnected([]()    { Serial.println("Connected!"); })
        .onDisconnected([]() { Serial.println("Disconnected!");  })
        .onPortal([]()       { Serial.println("Portal running..."); })
        .onTimeout([]()      { Serial.println("Portal closed for energy saving"); })
        .begin("My-ESP32-AP");
}
```

#### 2. 🛠️ Advanced API (Enum-based)

Perfect for State Machines using `switch-case`:

```cpp
void wifiStatusHandler(WiFiManager::WiFiState state) {
    switch(state) {
        case WiFiManager::CONNECTED:    Serial.println("Connected");    break;
        case WiFiManager::DISCONNECTED: Serial.println("Disconnected"); break;
        case WiFiManager::PORTAL_START:  Serial.println("Portal UI Up"); break;
        case WiFiManager::PORTAL_TIMEOUT: Serial.println("Auto-Closed");  break;
    }
}

void setup() {
    wifiManager.onStatusChange(wifiStatusHandler).begin();
}
```

#### 3. 🔌 Middleware Mode (Shared Dashboard) - *Pro*

If your project already has an `AsyncWebServer(80)`, use this to avoid Port 80 conflicts:

```cpp
AsyncWebServer server(80);

void setup() {
    // Register your dashboard
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello from Dashboard!");
    });

    // Let WiFiManager use your server
    wifiManager.useServer(&server).begin("My-ESP32-AP");
    
    server.begin();
}
```

> [!WARNING]
> If you start your own WebServer on Port 80 **without** using `.useServer(&server)`, the Captive Portal will fail to start due to port conflict.

### ⚙️ Advanced Configuration

Customize default settings in [WM_Config.h](file:///d:/esp32/esp32-wifiManager/lib/WiFiManager/WM_Config.h):
- **WiFi:** Change default AP name (`WM_DEFAULT_AP_NAME`).
- **Hardware:** Configure status LED pin and active level.
- **Timing:** Adjust timeouts and blink intervals for all states.

---

### 🔌 API Reference

- `begin(apName, apPassword)`: Start WiFi/Portal systems.
- `setStatusLED(pin, activeLow)`: Configure status indication.
  - **Portal Mode:** Solid ON.
  - **Connecting:** Fast blink.
  - **Connected (Normal):** Slow blink (3s).
  - **Connected (Sleep):** Very slow heartbeat (5s).
- `setLEDActiveTime(ms)`: LED "ON" duration during blinks (Default: 200ms).
- `enableStatusLED(bool)`: Enable/Disable library control of the LED.
- `onSleepChange(callback)`: Triggered on Modem Sleep state change.
- `resetSettings()`: Clear saved WiFi credentials and Preferences.
- `now()`: Returns "YYYY-MM-DD HH:mm:ss".
- `date()`: Returns "YYYY-MM-DD".
- `time()`: Returns "HH:mm:ss".
- `getTimestamp()`: Returns Unix Timestamp.
- `isTimeSynced()`: Check if NTP sync is successful.

### 👨‍💻 Developer Guide

To modify the Portal UI:
1. Edit files in `data/` (`index.html`, `style.css`, `script.js`).
2. Run the Python generation script:
   ```bash
   python generate_assets.py
   ```
3. The script will inline assets and update `lib/WiFiManager/WebAssets.h` automatically.

---

## ภาษาไทย

ไลบรารีจัดการ WiFi สำหรับ ESP32 ที่ออกแบบมาให้ **ใช้งานง่าย (Eazy)**, **รวดเร็ว (Fast)**, **ประหยัดพลังงาน (Low Energy)** และ **ไม่บล็อกการทำงานหลัก (Non-blocking)** โดยอิงตามมาตรฐาน FreeRTOS

### ✨ คุณสมบัติเด่น (Features)

- **Plug-and-Play:** เรียกใช้งานเพียงบรรทัดเดียว `wifiManager.begin()`
- **Smooth Scan:** ค้นหา WiFi แบบ Real-time รายการจะค่อยๆ ปรากฏขึ้นมาในหน้าเว็บ (UX สไตล์ Fomantic-UI)
- **Captive Portal:** ระบบเด้งหน้าตั้งค่าให้อัตโนมัติเมื่อเขื่อมต่อกับ ESP32 (รองรับ DNS Redirect)
- **Multi-Network Auto-connect:** จดจำเครือข่ายที่เคยเชื่อมต่อได้สูงสุด 3 อันดับล่าสุด
- **Low Energy:** เปิดโหมด Modem Sleep อัตโนมัติ และมีระบบปิด AP เองเมื่อไม่มีการใช้งาน (AP Timeout)
- **Non-blocking FreeRTOS:** การสแกนและจัดการ WiFi ทำงานเป็น Task เบื้องหลัง ไม่รบกวน CPU หลัก

### 🛠 การติดตั้ง (Installation)

เพียงเพิ่มบรรทัดนี้ใน `platformio.ini` ของคุณ:

```ini
lib_deps =
    https://github.com/noobtohero/esp32-wifiManager.git
```

**ไม่ต้องทำอะไรเพิ่ม!** ระบบจะจัดการ Lib อื่นๆ (AsyncWebServer, ArduinoJson) และหน้าเว็บให้คุณโดยอัตโนมัติ (Zero-Config)

### 📖 วิธีการใช้งาน (Quick Start)

Library นี้ออกแบบมาให้ใช้งานได้ 2 สไตล์ ตามความชอบครับ:

#### 1. 🚀 แบบง่าย (Simplified API) - *แนะนำ*

เน้นความคลีน ใช้ต่อกันเป็นลูกโซ่ (Fluent API) เหมาะกับงานทั่วไป:

```cpp
void setup() {
    wifiManager
        .setStatusLED()        // (เลือกใส่) ใช้ไฟ LED บนบอร์ดบอกสถานะอัตโนมัติ
        .onConnected([]()    { Serial.println("เน็ตมาแล้ว!"); })
        .onDisconnected([]() { Serial.println("WiFi หลุด!");  })
        .onPortal([]()       { Serial.println("รอตั้งค่า..."); })
        .onTimeout([]()      { Serial.println("ปิด Portal เองเพื่อประหยัดไฟ"); })
        .begin("My-ESP32-AP");
}
```

#### 2. 🛠️ แบบแอดวานซ์ (Enum-based API)

เหมาะสำหรับคนชอบเขียน State Machine หรือจัดการทุกสถานะในจุดเดียวด้วย `switch-case`:

```cpp
void wifiStatusHandler(WiFiManager::WiFiState state) {
    switch(state) {
        case WiFiManager::CONNECTED:    Serial.println("Connected");    break;
        case WiFiManager::DISCONNECTED: Serial.println("Disconnected"); break;
        case WiFiManager::PORTAL_START:  Serial.println("Portal UI Up"); break;
        case WiFiManager::PORTAL_TIMEOUT: Serial.println("Auto-Closed");  break;
    }
}

void setup() {
    wifiManager.onStatusChange(wifiStatusHandler).begin();
}
```

#### 3. 🔌 โหมด Middleware (ใช้งานร่วมกับ Dashboard) - *Pro*

หากโปรเจกต์ของคุณมี `AsyncWebServer(80)` อยู่แล้ว ให้ใช้โหมดนี้เพื่อป้องกันพอร์ตชนกัน:

```cpp
AsyncWebServer server(80);

void setup() {
    // สร้างหน้า Dashboard ของคุณ
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "สวัสดีจากหน้า Dashboard!");
    });

    // ให้ WiFiManager ไปใช้ Server ร่วมกับคุณ
    wifiManager.useServer(&server).begin("My-ESP32-AP");
    
    server.begin();
}
```

> [!WARNING]
> หากคุณสร้าง WebServer เองบนพอร์ต 80 โดย **ไม่ใช้** คำสั่ง `.useServer(&server)` หน้าตั้งค่า WiFi (Portal) จะไม่สามารถทำงานได้เนื่องจากพอร์ตชนกันครับ

### ⚙️ การตั้งค่าระดับสูง (Advanced Configuration)

คุณสามารถปรับแต่งค่าเริ่มต้นของระบบ (Default Settings) ได้ที่ไฟล์ [WM_Config.h](file:///d:/esp32/esp32-wifiManager/lib/WiFiManager/WM_Config.h) ภายในโฟลเดอร์ Library:

- **WiFi:** เปลี่ยนชื่อ AP เริ่มต้น (`WM_DEFAULT_AP_NAME`)
- **Hardware:** เปลี่ยนขา LED เริ่มต้น หรือค่า Invert
- **Timing:** ปรับเวลา Timeout หรือจังหวะการกะพริบของไฟในทุกสถานะ

---

### 🔌 API Reference

- `begin(apName, apPassword)`: เริ่มระบบ WiFi/Portal
- `setStatusLED(pin, activeLow)`: ตั้งค่าไฟบอกสถานะ
  - **Portal Mode:** ไฟติดค้าง (Solid ON)
  - **Connecting:** กะพริบเร็ว
  - **Connected (Normal):** กะพริบช้า ทุก 3 วินาที
  - **Connected (Sleep):** กะพริบช้ามาก ทุก 5 วินาที
- `setLEDActiveTime(ms)`: ตั้งเวลาที่ไฟ LED สว่างค้างตอนกะพริบ (Default: 200ms)
- `enableStatusLED(bool)`: เปิด/ปิดการทำงานของไฟ LED สถานะ
- `onSleepChange(callback)`: ดักจับตอนโหมดประหยัดพลังงานเปลี่ยนสถานะ
- `resetSettings()`: ล้างค่า WiFi และ Preferences ทั้งหมด
- `now()`: วันและเวลาปัจจุบัน ("YYYY-MM-DD HH:mm:ss")
- `date()`: วันที่ปัจจุบัน ("YYYY-MM-DD")
- `time()`: เวลาปัจจุบัน ("HH:mm:ss")
- `getTimestamp()`: Unix Timestamp
- `isTimeSynced()`: เช็คว่าซิงค์เวลากับ NTP สำเร็จแล้วหรือยัง

### 👨‍💻 สำหรับนักพัฒนา (Developer Guide)

หากต้องการแก้ไขหน้าเว็บ Portal:
1. แก้ไขไฟล์ในโฟลเดอร์ `data/` (`index.html`, `style.css`, `script.js`)
2. รัน Script อัปเดตไฟล์ Header:
   ```bash
   python generate_assets.py
   ```
3. Script จะสร้างไฟล์ `lib/WiFiManager/WebAssets.h` ให้ใหม่โดยอัตโนมัติ

---
Developed by **Antigravity AI (Google Deepmind)** 🧬
Managed by **NoobToHERO** 🛠️