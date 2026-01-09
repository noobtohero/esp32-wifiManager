# WiFiManager (Plug-and-Play) 🚀

A comprehensive WiFi management solution for ESP32. It features a modern, responsive Captive Portal with "Instant Feedback" connection testing, supports multiple SSID storage, and operates non-blocking via FreeRTOS.

[English](#english) | [ภาษาไทย](#ภาษาไทย)

---

## English

### ✨ Features

- **⚡ Instant Feedback Flow:** Tests credentials *before* restarting. No more reboot loops for wrong passwords!
- **📶 Multi-SSID Storage:** Remembers the last 3 connected networks and auto-connects to the available one.
- **📱 Modern Captive Portal:** 
    - Auto-redirect (iOS/Android/Windows).
    - Responsive UI with Semantic UI.
    - Signal Strength Icons and Security Indicators.
    - "Scanning..." animations and manual Refresh button.
- **🔄 Non-blocking Async Scan:** Fast, background WiFi scanning without freezing the device loop.
- **🛠 Zero-Dependency Frontend:** All HTML/CSS/JS assets are embedded into headers. No external LittleFS upload required.
- **🔌 Plug-and-Play:** Simple API with `wifiManager.begin()`.
- **🔋 Low Energy:** Integrated Modem Sleep and AP Timeout mechanism.
- **🕒 RTC Sync:** Automatic NTP time synchronization.

### 🔧 Technical Details

- **Filesystem:** Embedded Assets (generated via `generate_assets.py` from `data/` folder).
- **Core:** Runs on a dedicated FreeRTOS task.
- **Networking:**
    - Uses `WIFI_AP_STA` mode to allow simultaneous AP + Scanning.
    - **DHCP/DNS:** Forces IP `192.168.4.1` to ensure reliable Captive Portal redirection on all devices.
- **Configuration:** Centralized in `WM_Config.h`.

### 🛠 Frontend Development
If you modify any files in the `data/` folder (`index.html`, `style.css`, `script.js`), you **must** run the following command to update `WebAssets.h`:
```bash
python generate_assets.py
```


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

---

## ภาษาไทย

### ✨ คุณสมบัติเด่น

- **⚡ Instant Feedback:** ทดสอบการเชื่อมต่อ WiFi ทันทีที่กดปุ่ม Save รู้ผล Success/Fail ได้เลยโดย **ไม่ต้องรอรีบูตเครื่อง**
- **📶 เก็บชื่อ WiFi ได้ 3 ชื่อ:** ระบบจะจำ 3 ชื่อล่าสุดที่เคยต่อได้ และวนเช็ค Auto-connect ให้เองตามลำดับ
- **📱 หน้าเว็บทันสมัย:**
    - เด้งเข้าหน้าตั้งค่าอัตโนมัติ (Captive Portal)
    - UI สวยงาม มีบอกระดับสัญญาณ (Signal Strength) และแม่กุญแจบอกความปลอดภัย
    - มีปุ่ม Refresh และ Animation บอกสถานะการสแกน
- **🔄 สแกนไว ไม่ค้าง:** ใช้ระบบ Async Scan ทำงานเบื้องหลัง ไม่บล็อกการทำงานหลักของบอร์ด
- **🛠 ไม่ต้องอัพโหลดไฟล์:** ไฟล์เว็บทั้งหมดถูกแปลงเป็น Header ฝังใน Code แล้ว แฟลชปุ๊บใช้ได้ปั๊บ
- **🔋 ประหยัดพลังงาน:** ปิด AP อัตโนมัติเมื่อไม่มีการใช้งาน (Timeout)
- **🕒 ตั้งเวลาอัตโนมัติ:** ดึงเวลาจากเน็ต (NTP) ให้เองเมื่อต่อติด

### 🔧 ข้อมูลทางเทคนิค

- **Filesystem:** ไม่ต้องใช้ SPIFFS/LittleFS แยก (ใช้ Asset Embedding)
- **Core:** ทำงานแยก Thread ด้วย FreeRTOS Task
- **Networking:**
    - ใช้โหมด `WIFI_AP_STA` ทำให้ปล่อย Hotspot ไปด้วย และสแกน WiFi ไปด้วยได้
    - **DHCP/DNS:** บังคับ IP `192.168.4.1` เพื่อแก้ปัญหา Android/iOS บางรุ่นไม่เด้งหน้า Portal
- **Configuration:** ปรับแต่งค่าต่างๆ ได้ง่ายๆ ที่ไฟล์ `WM_Config.h`

### 🛠 การพัฒนาหน้าเว็บ (Frontend)
หากมีการแก้ไขไฟล์ในโฟลเดอร์ `data/` (`index.html`, `style.css`, `script.js`) คุณ **จำเป็น** ต้องรันสคริปต์เพื่ออัปเดตไฟล์ `WebAssets.h` ทุกครั้ง:
```bash
python generate_assets.py
```


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
