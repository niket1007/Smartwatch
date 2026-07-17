#pragma once

#include "globals.h"

struct Weather_Details {
    String temp;
    String hi;
    String lo;
    String hum;
    String rain;
    String wind;
    String txt;
    String loc;
};
extern Weather_Details wd_notif;

extern void send_weather_update_command();
extern void update_weather_details(
    const String &temp, const String &hi, const String &lo, const String &hum, 
    const String &rain, const String &wind, const String &txt, const String &loc);
extern void update_weather_screen_ui();