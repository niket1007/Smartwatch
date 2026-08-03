#pragma once

#include "Common/constants.h"
#include "Display/display_driver.h"
#include "Graphics/graphics.h"
#include "Battery/battery_manager.h"
#include "I2C/i2c_manager.h"
#include "Storage/nvs_manager.h"
#include "WiFi/wifi_manager.h"
#include "RTC/rtc_manager.h"
#include "Touch/touch_manager.h"
#include "Screen/screen_manager.h"

extern DisplayDriver display_driver;
extern Graphics graphics;
extern BatteryManager battery_manager;
extern I2CManager i2c_manager;
extern NVSManager nvs_manager;
extern WiFiManager wifi_manager;
extern RTCManager rtc_manager;
extern TouchManager touch_manager;
extern ScreenManager screen_manager;

extern TaskHandle_t gui_task_handle;
extern TaskHandle_t background_task_handle;
extern TaskHandle_t time_sync_handle;
extern TaskHandle_t touch_task_handle;