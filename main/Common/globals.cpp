#include "globals.h"

#include "esp_timer.h"

DisplayManager display_manager;
ScreenManager screen_manager;
BluetoothManager bluetooth_manager;
NVSManager nvs_manager;
I2CManager i2c_manager;
RTCManager rtc_manager;
WiFiManager wifi_manager;
BatteryManager battery_manager;
PowerSaverManager power_saver_manager;

TaskHandle_t gui_task_handle = nullptr;
TaskHandle_t background_task_handle = nullptr;
TaskHandle_t time_sync_handle = nullptr;
TaskHandle_t touch_task_handle = nullptr;

esp_pm_lock_handle_t cpu_freq_lock = nullptr;
esp_pm_lock_handle_t no_sleep_lock = nullptr;


int64_t millis(void)
{
    // esp_timer_get_time return microseconds
    return esp_timer_get_time() / 1000;
}