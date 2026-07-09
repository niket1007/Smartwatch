#pragma once

#include <Arduino.h>
#include "pin_config.h"
#include <Wire.h>
#include "HWCDC.h"
#include "Arduino_GFX_Library.h"

extern "C" {
  #include "ui/ui.h"
  #include "ui/actions.h"
}

extern TaskHandle_t task_gui_handle;

extern HWCDC usb_serial;
extern SemaphoreHandle_t i2c_mutex;
extern Arduino_GFX *gfx;
extern uint32_t ble_passkey;
// 0 -> Nothing; 
// 1 -> show passkey display; 
// 2 -> Display Status Paired; 
// 3-> Display Status Paired Failed;
// 4 -> Move to Home Screen;
extern volatile uint32_t show_passkey_display; 

extern uint32_t sleep_count;

extern void turn_off_screen();
extern void turn_on_screen();
extern bool lvgl_get_touch(int16_t &x, int16_t &y);
int get_battery_percentage();
void set_battery_percentage(int percentage);
bool is_battery_charging();
void set_battery_charging(bool charging);
