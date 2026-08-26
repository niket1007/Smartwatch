#pragma once

#include "constants.h"
#include "Screen/screen_manager.h"
#include "Bluetooth/bluetooth_manager.h"
#include "Storage/nvs_manager.h"
#include "Display/display_manager.h"
#include "I2C/i2c_manager.h"
#include "RTC/rtc_manager.h"
#include "WiFi/wifi_manager.h"
#include "Battery/battery_manager.h"
#include "Power_Saver/power_saver_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_pm.h"

#define BOOT_BUTTON_PIN 0
#define BUTTON_ACTIVE_LEVEL 0

extern DisplayManager display_manager;
extern ScreenManager screen_manager;
extern BluetoothManager bluetooth_manager;
extern NVSManager nvs_manager;
extern I2CManager i2c_manager;
extern RTCManager rtc_manager;
extern WiFiManager wifi_manager;
extern BatteryManager battery_manager;
extern PowerSaverManager power_saver_manager;

extern TaskHandle_t gui_task_handle;
extern TaskHandle_t background_task_handle;
extern TaskHandle_t time_sync_handle;
extern TaskHandle_t touch_task_handle;

int64_t millis(void);

extern esp_pm_lock_handle_t cpu_freq_lock;
extern esp_pm_lock_handle_t no_sleep_lock;