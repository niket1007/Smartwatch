#pragma once

#include "constants.h"
#include "Screen/screen_manager.h"
#include "Bluetooth/bluetooth_manager.h"
#include "Storage/nvs_manager.h"
#include "Display/display_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BOOT_BUTTON_PIN 0
#define BUTTON_ACTIVE_LEVEL 0

extern DisplayManager display_manager;
extern ScreenManager screen_manager;
extern BluetoothManager bluetooth_manager;
extern NVSManager nvs_manager;

extern TaskHandle_t gui_task_handle;
extern TaskHandle_t background_task_handle;
extern TaskHandle_t time_sync_handle;
extern TaskHandle_t touch_task_handle;

int64_t millis(void);