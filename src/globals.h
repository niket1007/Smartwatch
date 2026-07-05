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


extern HWCDC usb_serial;
extern SemaphoreHandle_t i2c_mutex;
extern Arduino_GFX *gfx;

extern bool lvgl_get_touch(int16_t &x, int16_t &y);
int get_battery_percentage();
void set_battery_percentage(int percentage);
bool is_battery_charging();
void set_battery_charging(bool charging);
