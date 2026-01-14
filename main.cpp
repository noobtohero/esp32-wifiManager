#include "src/WiFiManager.h"
#include <Arduino.h>
#include <WiFiManager.h>

// ตัวอย่างการใช้งาน ESP32 WiFi Manager แบบ Refactored
// 1. แยกไฟล์ HTML/CSS ใน folder /data (อัปโหลดผ่าน LittleFS)
// 2. ใช้ FreeRTOS จัดการ WiFi สแกนเบื้องหลัง
// 3. รองรับ Captive Portal (เด้งเข้าหน้า Config อัตโนมัติ)
// 4. Low Energy ด้วย Modem Sleep

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n[APP] Booting...");

  // ท่าที่ "สั้นที่สุด" (One-Liner with Auto-LED)
  wifiManager.begin("MyAP");
}

/*
   Tips การใช้งานสำหรับระบบประหยัดพลังงาน:
   1. สำหรับงานที่ต้องการ Low Latency หรือเปิด Web Server / OTA Update:
      wifiManager.setSleep(false); // ปิด Sleep Mode เพื่อให้ตอบสนองเร็ว

   2. สำหรับงานส่งข้อมูลแล้วจบไป (เช่น Telegram Bot, Line Notify):
      wifiManager.setSleep(false); // ปิดชั่วคราวเพื่อให้ส่งไว
      // ... โค้ดส่งข้อมูล ...
      wifiManager.setSleep(true);  // เปิดกลับคืนเพื่อประหยัดไฟ
*/

void loop() {
  // โค้ดใน loop() จะไม่ถูกบล็อกโดยการทำงานของ WiFi
  static unsigned long lastTick = 0;
  if (millis() - lastTick > 5000) {
    lastTick = millis();
    Serial.printf("[APP] Time: %s | Free Heap: %u\n", wifiManager.now().c_str(),
                  ESP.getFreeHeap());
  }
}
