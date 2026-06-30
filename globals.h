#pragma once

#include <Arduino.h>
#include "pin_config.h"
#include <Wire.h>
#include "HWCDC.h"

extern "C" {
  #include "src/ui/ui.h"
  #include "src/ui/actions.h"
}


extern HWCDC usb_serial;
extern SemaphoreHandle_t i2c_mutex;

int get_battery_percentage();
void set_battery_percentage(int percentage);

bool is_battery_charging();
void set_battery_charging(bool charging);