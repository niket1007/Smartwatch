#pragma once

#include "globals.h"
#include "XPowersLib.h"

extern void power_init();
extern int get_accurate_battery_percentage();
extern void update_battery_ui();
extern void read_battery_sensor();