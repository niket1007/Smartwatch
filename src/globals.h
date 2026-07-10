#pragma once

#include <Arduino.h>
#include "pin_config.h"
#include <Wire.h>
#include "HWCDC.h"
#include "Arduino_GFX_Library.h"
#include <Preferences.h>
extern "C" {
  #include "ui/ui.h"
  #include "ui/actions.h"
}

// WIFI Global Variables
struct Wifi_Global_Variable {
  String ssid;
  String password;
  String wifi_list;
};
extern Wifi_Global_Variable wifi_gv; 

struct Task_State_Variables {
  /*
    0 -> Idle
    1 -> WiFi Scanning Started
    2 -> WiFi Scanning completed, GUI Starts
    3 -> Time sync
  */
  volatile int wifi_refresh;
};
extern Task_State_Variables ts_var;

extern TaskHandle_t task_gui_handle;
extern TaskHandle_t task_background_handle;

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
extern Preferences preferences;

extern void turn_off_screen();
extern void turn_on_screen();
extern bool lvgl_get_touch(int16_t &x, int16_t &y);
int get_battery_percentage();
void set_battery_percentage(int percentage);
bool is_battery_charging();
void set_battery_charging(bool charging);
