#include "globals.h"

DisplayDriver display_driver;
Graphics graphics;
BatteryManager battery_manager;
I2CManager i2c_manager;
NVSManager nvs_manager;
WiFiManager wifi_manager;
RTCManager rtc_manager;
TouchManager touch_manager;
ScreenManager screen_manager;

TaskHandle_t gui_task_handle = nullptr;
TaskHandle_t background_task_handle = nullptr;
TaskHandle_t time_sync_handle = nullptr;
TaskHandle_t touch_task_handle = nullptr;
