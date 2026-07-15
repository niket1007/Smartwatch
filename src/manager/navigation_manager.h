#pragma once

#include "globals.h"
#include <lvgl.h>

#define HOME_SCREEN 0
#define WEATHER_SCREEN 1
#define NAVIGATION_SCREEN 2
#define CALENDAR_SCREEN 3
#define MUSIC_SCREEN 4
#define SETTINGS_SCREEN -10
#define NOTIFICATION_SCREEN -20
#define BLE_PASSKEY_SCREEN -30

extern lv_obj_t* get_screen(int number);
extern void action_navigate_gesture(lv_event_t * e);
void navigate_to_screen(int screen);