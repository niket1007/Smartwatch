#pragma once

#include <Arduino.h>
#include "pin_config.h"
#include <Wire.h>
#include "HWCDC.h"
#include "Arduino_GFX_Library.h"
#include <Preferences.h>
#include <ArduinoJson.h>
extern "C" {
  #include "ui/ui.h"
  #include "ui/actions.h"
}

// WIFI Global Variables
struct Global_Variables {
  String ssid;
  String password;
  String wifi_list;
  bool ble_is_powered_on;
  bool ble_is_subscribed;
  bool is_screen_active;
  int active_screen_id;
  uint32_t ble_passkey;
};
extern Global_Variables gv; 

struct Task_State_Variables {
  /*
    0 -> Idle
    Wifi refresh button clicked
      1 -> WiFi Scanning Started
      2 -> WiFi Scanning completed, GUI dropdown update
      Flow: 0 -> 1 -> 2 -> 0
  */
  volatile int wifi_refresh;

  /* 
    0 -> Nothing
    1 -> show passkey display
    2 -> Display Status Paired
    3-> Display Status Paired Failed
    4 -> Move to Home Screen and invoke battery and weather init
    Flow: 0 -> 1 -> 2/3 -> 4 -> 0
  */
  volatile uint32_t show_passkey_display;

  /*
    0 -> Nothing
    1 -> BLE Notification updated for incoming/outgoing, then update the UI
    2 -> Notification updated for end, then navigate back to Home
    Flow: 0 -> 1 -> 2 -> 0
  */
  volatile uint32_t show_call_notif_screen;

  /*
    0 -> Nothing
    1 -> Refresh button clicked
    2 -> Send the updated weather fetch command
    3 -> Weather notification received
    Flow: 0 -> 1 -> 2 -> 3 -> 0  
  */
  volatile uint32_t weather_refresh;

  /*
    0 -> Nothing
    1 -> Update the battery and weather
    Flow: 0 -> 1 -> 0
  */
  volatile bool bluetooth_battery_weather_init;

  /*
    0 -> Nothing
    1 -> Send the accept call signal
    Flow: 0 -> 1 -> 0
  */
  volatile int accept_call_signal_init; 

  /*
    0 -> Nothing
    1 -> Send the reject call signal
    Flow: 0 -> 1 -> 0
  */
  volatile int reject_call_signal_init;

  /*
    false: nothing/no music update
    true: music update
  */
  volatile bool is_music_event_received;

  /*
    false: nothing/no nav update
    true: nav update
  */
  volatile bool is_nav_event_received;
};
extern Task_State_Variables ts_var;

extern TaskHandle_t task_gui_handle;
extern TaskHandle_t task_background_handle;

extern HWCDC usb_serial;
extern SemaphoreHandle_t i2c_mutex;
extern Arduino_GFX *gfx;

extern uint32_t sleep_count;

extern void turn_off_screen();
extern void turn_on_screen();
extern bool lvgl_get_touch(int16_t &x, int16_t &y);
int get_battery_percentage();
void set_battery_percentage(int percentage);
bool is_battery_charging();
void set_battery_charging(bool charging);

extern bool ble_send_to_phone(JsonDocument json);