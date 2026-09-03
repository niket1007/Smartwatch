#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Arduino_DriveBus_Library.h"
#include <Arduino.h>
#include "pin_config.h"
#include <Wire.h>
#include "HWCDC.h"
#include "Arduino_GFX_Library.h"
#include <Preferences.h>
#include <ArduinoJson.h>

#include "esp_pm.h"

// Constants
#include "constants.h"

// Managers
#include "Battery/battery_manager.h"
#include "LVGL/lvgl_manager.h"
#include "WiFi/wifi_manager.h"
#include "RTC/rtc_manager.h"
#include "Bluetooth/bluetooth_manager.h"
#include "Screen/screen_manager.h"
#include "Display/display_manager.h"
#include "PowerSaver/power_saver_manager.h"
#include "Storage/storage_manager.h"
// #include "Logs/log_manager.h"

#define BOOT_BUTTON_PIN 0

extern TaskHandle_t gui_task_handle;
extern TaskHandle_t background_task_handle;

extern HWCDC usb_serial;
extern SemaphoreHandle_t i2c_mutex;
extern Arduino_CO5300 *gfx;
extern Arduino_DataBus *bus;

extern esp_pm_lock_handle_t sleep_lock;

extern bool light_sleep_enable;

extern bool lvgl_get_touch(int16_t &x, int16_t &y);

extern BatteryManager battery_manager;
extern WifiManager wifi_manager;
extern RTCManager rtc_manager;
extern BluetoothManager bluetooth_manager;
extern ScreenManager screen_manager;
extern DisplayManager display_manager;
extern PowerSaverManager power_saver_manager;
extern StorageManager storage_manager;
// extern Log log;