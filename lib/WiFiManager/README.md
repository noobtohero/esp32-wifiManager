# WiFiManager (Plug-and-Play) 🚀

A comprehensive WiFi management solution for ESP32 that uses FreeRTOS tasks to handle connections in the background, including a sleek captive portal for initial configuration and automatic NTP time synchronization.

[English](#english) | [ภาษาไทย](#ภาษาไทย)

---

## English

### ✨ Features

- **Plug-and-Play:** Simple API with `wifiManager.begin()`.
- **Captive Portal:** Automatic popup for WiFi configuration.
- **Non-blocking:** Powered by FreeRTOS, doesn't freeze the main loop.
- **Auto-connect:** Remembers multiple networks.
- **Low Energy:** Integrated Modem Sleep support.
- **RTC Sync:** Automatic NTP time synchronization.

### 🛠 Installation (PlatformIO)

Add to your `platformio.ini`:

```ini
lib_deps =
    NoobToHERO/WiFiManager
```

### 📖 Quick Start

```cpp
#include <WiFiManager.h>

void setup() {
    wifiManager
        .setStatusLED() // Optional: use built-in LED
        .begin("My-ESP32-Portal");
}

void loop() {
    // Main code runs smoothly here!
}
```

### ⚙️ Configuration

Settings can be adjusted in [src/WM_Config.h](src/WM_Config.h).

---

## ภาษาไทย

### ✨ คุณสมบัติ

- **Plug-and-Play:** ใช้งานง่ายด้วย `wifiManager.begin()`.
- **Captive Portal:** ระบบเด้งหน้าตั้งค่า WiFi อัตโนมัติ
- **Non-blocking:** ใช้ FreeRTOS ทำงานเบื้องหลัง ไม่บล็อก loop หลัก
- **Auto-connect:** จดจำและเชื่อมต่อ WiFi อัตโนมัติ
- **Low Energy:** รองรับโหมดประหยัดพลังงาน
- **RTC Sync:** ซิงค์เวลา NTP ให้อัตโนมัติ

### 🛠 การติดตั้ง (PlatformIO)

เพิ่มใน `platformio.ini`:

```ini
lib_deps =
    NoobToHERO/WiFiManager
```

### 📖 เริ่มต้นใช้งาน

```cpp
#include <WiFiManager.h>

void setup() {
    wifiManager
        .setStatusLED() // เลือกใช้ไฟ LED บนบอร์ดบอกสถานะ
        .begin("My-ESP32-Portal");
}

void loop() {
    // โค้ดหลักของคุณทำงานได้ตามปกติที่นี่!
}
```

### ⚙️ การตั้งค่า

สามารถปรับแต่งค่าต่างๆ ได้ที่ไฟล์ [src/WM_Config.h](src/WM_Config.h)
