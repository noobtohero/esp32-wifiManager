# ESP32 WiFi Manager (Plug-and-Play) 🚀

**Latest Release:** `v1.3.0` (Middleware Mode Support)

A high-performance, non-blocking WiFi Manager for ESP32.

[English](#english) | [ภาษาไทย](#ภาษาไทย)

---

## English

A WiFi management library for ESP32 designed to be **Easy to use**, **Fast**, **Low Energy**, and **Non-blocking**, powered by FreeRTOS.

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

### 📂 Structure

- **[lib/WiFiManager](lib/WiFiManager)**: The core library.
- **[lib/WiFiManager/examples](lib/WiFiManager/examples)**: Usage examples for Arduino IDE & PlatformIO (Basic, Dashboard, OTA).
- **[lib/WiFiManager/WM_Config.h](lib/WiFiManager/WM_Config.h)**: Global configuration.

For full documentation, please visit the [Library README](lib/WiFiManager/README.md).

---

## ภาษาไทย

ไลบรารีจัดการ WiFi สำหรับ ESP32 ที่ออกแบบมาให้ **ใช้งานง่าย**, **รวดเร็ว**, **ประหยัดพลังงาน** และ **ไม่บล็อกการทำงานหลัก**

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

### 📂 โครงสร้างโปรเจกต์

- **[lib/WiFiManager](lib/WiFiManager)**: ตัว Library หลัก
- **[lib/WiFiManager/examples](lib/WiFiManager/examples)**: ตัวอย่างการใช้งาน
- **[lib/WiFiManager/WM_Config.h](lib/WiFiManager/WM_Config.h)**: ไฟล์ตั้งค่าระบบ

อ่านเอกสารฉบับเต็มได้ที่ [Library README](lib/WiFiManager/README.md)

---

### ⚠️ Note for Developers
When editing frontend source files in the `data/` folder, you must run:
`python generate_assets.py` 
to update the embedded header file (`WebAssets.h`).

---


---

Developed by **Antigravity AI (Google Deepmind)** 🧬
Managed by **NoobToHERO** 🛠️
