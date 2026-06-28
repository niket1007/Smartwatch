#pragma once

#include <Arduino.h>
#include "pin_config.h"
#include <Wire.h>
#include "HWCDC.h"

extern "C" {
  #include "src/ui/ui.h"
  #include "src/ui/actions.h"
}

// Below variables will be used in most of the files
extern HWCDC USBSerial;
extern SemaphoreHandle_t i2c_mutex;
extern volatile int global_battery_percentage;
extern volatile bool global_is_charging;