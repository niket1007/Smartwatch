#pragma once

#include "globals.h"
#include "XPowersLib.h"

extern XPowersAXP2101 power;

extern void power_init();
extern void update_battery_ui();
extern void read_battery_sensor();
void send_battery_related_information();