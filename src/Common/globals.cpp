#include "globals.h"

HWCDC usb_serial;
SemaphoreHandle_t i2c_mutex;

esp_pm_lock_handle_t sleep_lock;

TaskHandle_t gui_task_handle = nullptr;
TaskHandle_t background_task_handle = nullptr;

BatteryManager battery_manager;
WifiManager wifi_manager;
RTCManager rtc_manager;
BluetoothManager bluetooth_manager;