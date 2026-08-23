#include "globals.h"

#include "esp_timer.h"

DisplayManager display_manager;
ScreenManager screen_manager;
BluetoothManager bluetooth_manager;
NVSManager nvs_manager;

TaskHandle_t gui_task_handle = nullptr;
TaskHandle_t background_task_handle = nullptr;
TaskHandle_t time_sync_handle = nullptr;
TaskHandle_t touch_task_handle = nullptr;


int64_t millis(void)
{
    // esp_timer_get_time return microseconds
    return esp_timer_get_time() / 1000;
}