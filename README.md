# ESP32 WiFi Manager (Plug-and-Play) 🚀

**Latest Release:** `v0.2.0` (Smart Retry & Non-Blocking Workflow)

A high-performance, non-blocking WiFi Manager for ESP32.

[English](#english) | [ภาษาไทย](#ภาษาไทย) | [**คู่มือสำหรับมือใหม่ (Thai Guide)**](GUIDE_TH.md)

---

## English

A WiFi management library for ESP32 designed to be **Easy to use**, **Fast**, **Low Energy**, and **Non-blocking**, powered by FreeRTOS.

### ✨ Features

- **🛡️ Smart Retry System:** 
    - **Multi-Pass Boot Retry:** Persistent connection attempts with rest periods to bypass "Association refused" errors.
    - **Intelligent Backoff:** Cooldown delays between retries to improve router compatibility.
- **🔄 No-Reboot Flow:** Connect to WiFi and close the portal gracefully without restarting the device.
- **🧹 Auto-Deduplication:** Automatically removes duplicate SSIDs and shifts unique networks to prioritize the most recent ones.
- **🕒 RTC Sync:** Automatic NTP time synchronization.

### 🛠 Installation

#### 1. PlatformIO
Add this to your `platformio.ini`:
```ini
lib_deps =
    https://github.com/noobtohero/esp32-wifiManager.git
```

#### 2. Arduino IDE
1. Download this repository as a `.zip` file.
2. In Arduino IDE, go to **Sketch** -> **Include Library** -> **Add .ZIP Library...**.
3. Select the downloaded file.

### 🔧 Technical Details

- **Filesystem:** Embedded Assets (generated via `generate_assets.py` from `data/` folder).
- **Core:** Runs on a dedicated FreeRTOS task.
- **Networking:**
    - Uses `WIFI_AP_STA` mode to allow simultaneous AP + Scanning.
    - **DHCP/DNS:** Forces IP `192.168.4.1` to ensure reliable Captive Portal redirection on all devices.
- **Configuration:** Centralized in `src/WM_Config.h`.

### 📖 Quick Start

```cpp
#include <WiFiManager.h>

void setup() {
    // Configures AP Name & Optional Password
    wifiManager
        .setStatusLED() // Optional: use built-in LED
        .begin("My-ESP32-Portal");
}

void loop() {
    // Main code runs smoothly here!
}
```

### 🧠 Middleware Mode (Shared WebServer)

If you have your own `WebServer` for a Dashboard or OTA, you can share the port 80 with WiFiManager. This saves RAM and prevents port conflicts.

```cpp
WebServer myServer(80);

void setup() {
    myServer.on("/hello", []() { myServer.send(200, "text/plain", "Hello!"); });

    wifiManager
        .useServer(&myServer) // Register your server
        .begin("My-Device-Portal");
    
    myServer.begin();
}
```
WiFiManager will automatically call `myServer.handleClient()` inside its FreeRTOS task, so you don't need to put it in your `loop()`.

---

## ภาษาไทย

ไลบรารีจัดการ WiFi สำหรับ ESP32 ที่ออกแบบมาให้ **ใช้งานง่าย**, **รวดเร็ว**, **ประหยัดพลังงาน** และ **ไม่บล็อกการทำงานหลัก** โดยใช้ความช่วยเหลือจาก FreeRTOS

### ✨ คุณสมบัติเด่น
- **⚡ Instant Feedback Flow:** Tests credentials *before* restarting. No more reboot loops for wrong passwords!
- **🛡️ ระบบ Smart Retry:** 
    - **Multi-Pass Retry:** พยายามเชื่อมต่อแบบวนรอบและมีช่วงพักตัว (Rest) เพื่อสู้กับอาการ Router ปฏิเสธการเชื่อมต่อ (Association refused)
    - **Intelligent Cooldown:** ระบบรอจังหวะที่เหมาะสมอัตโนมัติ ช่วยให้เกาะสัญญาณติดง่ายขึ้น
- **🔄 No-Reboot Flow:** เชื่อมต่อและสลับโหมดการทำงานได้ทันทีโดย **ไม่ต้องรีสตาร์ทเครื่อง** โปรแกรมทำงานต่อเนื่องได้ไม่สะดุด
- **🧹 ล้างชื่อซ้ำอัตโนมัติ:** ระบบจัดการ NVS อัจฉริยะ ลบ SSID ที่ซ้ำกันและจัดลำดับความสำคัญของเครือข่ายล่าสุดให้เอง
- **🕒 ตั้งเวลาอัตโนมัติ:** ดึงเวลาจากเน็ต (NTP) ให้เองเมื่อต่อติด

### 🛠 การติดตั้ง (Installation)

#### 1. PlatformIO
เพิ่มใน `platformio.ini`:
```ini
lib_deps =
    https://github.com/noobtohero/esp32-wifiManager.git
```

#### 2. Arduino IDE
1. ดาวน์โหลดโปรเจกต์นี้เป็นไฟล์ `.zip`
2. ไปที่เมนู **Sketch** -> **Include Library** -> **Add .ZIP Library...**
3. เลือกไฟล์ที่ดาวน์โหลดมา

### 🔧 ข้อมูลทางเทคนิค

- **Filesystem:** ไม่ต้องใช้ SPIFFS/LittleFS แยก (ใช้ Asset Embedding)
- **Core:** ทำงานแยก Thread ด้วย FreeRTOS Task
- **Networking:**
    - ใช้โหมด `WIFI_AP_STA` ทำให้ปล่อย Hotspot ไปด้วย และสแกน WiFi ไปด้วยได้
    - **DHCP/DNS:** บังคับ IP `192.168.4.1` เพื่อแก้ปัญหา Android/iOS บางรุ่นไม่เด้งหน้า Portal
- **Configuration:** ปรับแต่งค่าต่างๆ ได้ง่ายๆ ที่ไฟล์ `src/WM_Config.h`

### 📖 เริ่มต้นใช้งาน

```cpp
#include <WiFiManager.h>

void setup() {
    wifiManager
        .setStatusLED() // เลือกใช้ไฟ LED บนบอร์ดบอกสถานะ
        .begin("ESP32-Smart-Portal"); // ชื่อ WiFi ที่จะปล่อย
}

void loop() {
    // โค้ดหลักของคุณทำงานได้ตามปกติที่นี่!
}
```

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

Developed by **Antigravity AI (Google Deepmind)** 🧬
Managed by **NoobToHERO** 🛠️
