#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include <Wire.h>
#include "HWCDC.h"
#include <SensorPCF85063.hpp>
#include <fonts/Orbitron_SemiBold_10.h>
#include <WiFi.h>
#include <time.h>

HWCDC usb_serial;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
  LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_GFX *gfx = new Arduino_CO5300(bus, LCD_RESET /* RST */,
                                      0 /* rotation */,  LCD_WIDTH, LCD_HEIGHT,
                                      22 /* col_offset1 */,
                                      0 /* row_offset1 */,
                                      0 /* col_offset2 */,
                                      0 /* row_offset2 */);

SensorPCF85063 rtc;

const char* ssid     = "Amit 1st Floor";
const char* password = "12345689";
// Timezone Settings (Example: UTC + 5:30 for India = 19800 seconds)
// Calculate yours: (Hours Offset * 3600) + (Minutes Offset * 60)
const long  gmtOffset_sec = 19800; 
const int   daylightOffset_sec = 0; // Set to 3600 if your country uses Daylight Saving Time
const char* ntp_server = "pool.ntp.org";
const char* backup_ntp_server = "time.nist.gov";

int hours = 0;
int minutes = 0;
int seconds = 0;


void setup(void) {
  usb_serial.begin(115200);
  usb_serial.println("First Simple Watch Code");

#ifdef GFX_EXTRA_PRE_INIT
  GFX_EXTRA_PRE_INIT();
#endif

  // Init Display
  if (!gfx->begin()) {
    usb_serial.println("gfx->begin() failed!");
  }

  // Now init RTC (Clock chip) - Make sure to pass the address!
  if (!rtc.begin(Wire, IIC_SDA, IIC_SCL)) {
      usb_serial.println("RTC failed!");
  } else {
      usb_serial.println("RTC OK!");
  }

  // --- NTP TIME SYNC SEQUENCE ---
  usb_serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  // Wait for connection (with a timeout so we don't get stuck forever)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    usb_serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    usb_serial.println("\nWi-Fi Connected!");
    
    // Initialize the NTP client
    configTime(gmtOffset_sec, daylightOffset_sec, ntp_server, backup_ntp_server);
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 15000)) {
      usb_serial.println("Failed to obtain time from NTP server.");
    } else {
      usb_serial.println("NTP Time Fetched Successfully!");
      
      // Pass the fetched time into the Hardware RTC chip
      // Note: tm_year is years since 1900, tm_mon is 0-11
      rtc.setDateTime(timeinfo.tm_year + 1900, 
                      timeinfo.tm_mon + 1, 
                      timeinfo.tm_mday, 
                      timeinfo.tm_hour, 
                      timeinfo.tm_min, 
                      timeinfo.tm_sec);
                      
      usb_serial.println("Hardware RTC Updated!");
    }
    
    // CRITICAL: Turn off Wi-Fi to save battery!
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    usb_serial.println("Wi-Fi Powered Down.");
    
  } else {
    usb_serial.println("\nWi-Fi Connection Failed. Relying on existing RTC time.");
  }

  gfx->fillScreen(0xFFFF);
  gfx->setFont(&Orbitron_SemiBold10pt7b);

  // Top border
  gfx->fillRect(35, 35, 350, 430, 0x0000);

  gfx->setCursor(140, 80);
  gfx->setTextColor(0xFFFF);
  gfx->setTextSize(2);
  gfx->println("SMART");

  gfx->setCursor(80, 120);
  gfx->setTextColor(0xFFFF);
  gfx->setTextSize(2);
  gfx->println("WATCH");
}

void loop() {
  // --- 1. TIME LOGIC ---
  RTC_DateTime datetime = rtc.getDateTime();
  hours = datetime.getHour();
  minutes = datetime.getMinute();
  seconds = datetime.getSecond();
  
  char timeString[9]; 
  sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);

  gfx->fillRect(90, 195, 220, 50, 0x0000);
  gfx->setCursor(120, 220);
  gfx->setTextColor(0xFFFF);
  gfx->setTextSize(2);
  gfx->println(timeString);

  delay(1000);
}